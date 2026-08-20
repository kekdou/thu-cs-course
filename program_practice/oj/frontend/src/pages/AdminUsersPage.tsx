import { useEffect, useState } from "react";
import { RefreshCw } from "lucide-react";
import { api, friendlyError, type Role, type User } from "../api/client";
import ErrorMessage from "../components/ErrorMessage";
import ConfirmDialog from "../components/ConfirmDialog";

type PendingUserChange = {
  user: User;
  role: Role;
  isActive: boolean;
} | null;

export default function AdminUsersPage() {
  const [items, setItems] = useState<User[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [message, setMessage] = useState<string | null>(null);
  const [pendingChange, setPendingChange] = useState<PendingUserChange>(null);

  async function load() {
    setError(null);
    try {
      setItems((await api.users()).items);
    } catch (err) {
      setError(friendlyError(err, "用户列表加载失败，请确认你使用管理员账号登录"));
    }
  }

  async function save(user: User, role: Role, isActive: boolean) {
    setError(null);
    setMessage(null);
    try {
      const updated = await api.updateUser(user.id, role, isActive);
      setItems((current) => current.map((item) => (item.id === updated.id ? updated : item)));
      setMessage(`${updated.username} 已更新`);
    } catch (err) {
      setError(friendlyError(err, "用户保存失败，请确认角色合法，且没有禁用当前管理员自己"));
    }
  }

  useEffect(() => {
    void load();
  }, []);

  return (
    <section>
      <div className="page-head">
        <div><h1>用户管理</h1></div>
        <button onClick={load} title="刷新"><RefreshCw size={18} />刷新</button>
      </div>
      <ErrorMessage message={error} onClose={() => setError(null)} />
      {message && <div className="notice success">{message}</div>}
      {items.length === 0 ? <div className="notice">暂无用户</div> : (
        <div className="table-wrap">
          <table className="users-table">
            <colgroup>
              <col className="col-user" />
              <col className="col-role" />
              <col className="col-time" />
              <col className="col-active" />
            </colgroup>
            <thead><tr><th>用户名</th><th>角色</th><th>创建时间</th><th>启用</th></tr></thead>
            <tbody>
              {items.map((user) => (
                <tr key={user.id}>
                  <td className="strong-cell">{user.username}</td>
                  <td>
                    <select className="role-select" value={user.role} onChange={(event) => setPendingChange({ user, role: event.target.value as Role, isActive: user.is_active })}>
                      <option value="student">student</option>
                      <option value="teacher">teacher</option>
                      <option value="admin">admin</option>
                    </select>
                  </td>
                  <td>{new Date(user.created_at).toLocaleString()}</td>
                  <td>
                    <label className="switch">
                      <input type="checkbox" checked={user.is_active} onChange={(event) => setPendingChange({ user, role: user.role, isActive: event.target.checked })} />
                      <span>{user.is_active ? "启用" : "禁用"}</span>
                    </label>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
      {pendingChange && (
        <ConfirmDialog
          title="修改用户"
          message={`确认将 ${pendingChange.user.username} 修改为 ${pendingChange.role}，并${pendingChange.isActive ? "启用" : "禁用"}该账号吗？`}
          confirmText="确认修改"
          danger={!pendingChange.isActive}
          onConfirm={() => { void save(pendingChange.user, pendingChange.role, pendingChange.isActive); setPendingChange(null); }}
          onCancel={() => setPendingChange(null)}
        />
      )}
    </section>
  );
}
