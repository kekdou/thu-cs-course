import { useEffect, useState } from "react";
import { DatabaseBackup, RefreshCw, RotateCcw } from "lucide-react";
import { api, friendlyError, type Backup } from "../api/client";
import ErrorMessage from "../components/ErrorMessage";
import ConfirmDialog from "../components/ConfirmDialog";

export default function AdminBackupsPage() {
  const [items, setItems] = useState<Backup[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [message, setMessage] = useState<string | null>(null);
  const [restoreId, setRestoreId] = useState<string | null>(null);

  async function load() {
    setError(null);
    try {
      setItems(await api.backups());
    } catch (err) {
      setError(friendlyError(err, "备份列表加载失败，请确认你使用管理员账号登录"));
    }
  }

  async function create() {
    setError(null);
    setMessage(null);
    try {
      const backup = await api.createBackup();
      setMessage(`备份已创建：${backup.backup_id}`);
      await load();
    } catch (err) {
      setError(friendlyError(err, "创建备份失败，请确认数据库文件可访问且备份目录可写"));
    }
  }

  async function restore(id: string) {
    setError(null);
    setMessage(null);
    try {
      await api.restoreBackup(id);
      setMessage(`已从 ${id} 恢复`);
      await load();
    } catch (err) {
      setError(friendlyError(err, "恢复失败，请确认备份完整且 manifest.json 有效"));
    }
  }

  useEffect(() => {
    void load();
  }, []);

  return (
    <section>
      <div className="page-head">
        <div><h1>备份恢复</h1></div>
        <div className="actions">
          <button onClick={load} title="刷新"><RefreshCw size={18} />刷新</button>
          <button className="primary" onClick={create}><DatabaseBackup size={18} />创建备份</button>
        </div>
      </div>
      <ErrorMessage message={error} onClose={() => setError(null)} />
      {message && <div className="notice success">{message}</div>}
      {items.length === 0 ? <div className="notice">暂无备份</div> : (
        <div className="table-wrap">
          <table className="backups-table">
            <colgroup>
              <col className="col-title" />
              <col className="col-time" />
              <col className="col-actions" />
            </colgroup>
            <thead><tr><th>备份编号</th><th>创建时间</th><th></th></tr></thead>
            <tbody>
              {items.map((backup) => (
                <tr key={backup.backup_id}>
                  <td className="mono">{backup.backup_id}</td>
                  <td>{new Date(backup.created_at).toLocaleString()}</td>
                  <td><button className="icon-button" title="恢复" onClick={() => setRestoreId(backup.backup_id)}><RotateCcw size={18} /></button></td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
      {restoreId && (
        <ConfirmDialog
          title="恢复备份"
          message={`确认从备份 ${restoreId} 恢复数据吗？当前数据将被备份内容替换`}
          confirmText="确认恢复"
          danger
          onConfirm={() => { void restore(restoreId); setRestoreId(null); }}
          onCancel={() => setRestoreId(null)}
        />
      )}
    </section>
  );
}
