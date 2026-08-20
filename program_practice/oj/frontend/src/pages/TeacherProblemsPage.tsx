import { useEffect, useState } from "react";
import { Edit3, Plus, RefreshCw, Trash2, X } from "lucide-react";
import { api, friendlyError, type Problem, type ProblemListItem } from "../api/client";
import ErrorMessage from "../components/ErrorMessage";
import ConfirmDialog from "../components/ConfirmDialog";
import ProblemForm from "../components/ProblemForm";
import StatusBadge from "../components/StatusBadge";

export default function TeacherProblemsPage() {
  const [items, setItems] = useState<ProblemListItem[]>([]);
  const [editing, setEditing] = useState<Problem | null>(null);
  const [creating, setCreating] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [message, setMessage] = useState<string | null>(null);
  const [deleteId, setDeleteId] = useState<string | null>(null);

  async function load() {
    setError(null);
    try {
      setItems((await api.problems()).items);
    } catch (err) {
      setError(friendlyError(err, "题目列表加载失败，请确认你有教师或管理员权限"));
    }
  }

  async function openEdit(id: string) {
    setError(null);
    setCreating(false);
    try {
      setEditing(await api.problem(id));
    } catch (err) {
      setError(friendlyError(err, "题目详情加载失败，请刷新列表后重试"));
    }
  }

  async function remove(id: string) {
    setError(null);
    setMessage(null);
    try {
      await api.deleteProblem(id);
      setMessage(`题目 ${id} 已删除`);
      await load();
    } catch (err) {
      setError(friendlyError(err, "删除题目失败，请确认题目仍存在且你有权限"));
    }
  }

  async function create(problem: Problem) {
    setError(null);
    setMessage(null);
    try {
      await api.createProblem(problem);
      setCreating(false);
      setMessage(`题目 ${problem.id} 已创建`);
      await load();
    } catch (err) {
      setError(friendlyError(err, "创建题目失败，请检查题号是否重复、字段是否完整、测试点总分是否为 100"));
    }
  }

  async function update(problem: Problem) {
    if (!editing) return;
    setError(null);
    setMessage(null);
    try {
      await api.updateProblem(editing.id, problem);
      setEditing(null);
      setMessage(`题目 ${editing.id} 已更新`);
      await load();
    } catch (err) {
      setError(friendlyError(err, "更新题目失败，请检查字段格式、测试点编号和分值总和"));
    }
  }

  useEffect(() => {
    void load();
  }, []);

  return (
    <section>
      {!creating && !editing && (
        <div className="page-head">
          <div><h1>题目管理</h1></div>
          <div className="actions">
            <button onClick={load} title="刷新"><RefreshCw size={18} />刷新</button>
            <button className="primary" onClick={() => { setCreating(true); setEditing(null); }}><Plus size={18} />新建</button>
          </div>
        </div>
      )}
      <ErrorMessage message={error} onClose={() => setError(null)} />
      {message && <div className="notice success">{message}</div>}

      {(creating || editing) && (
        <div className="editor-panel">
          <div className="panel-head">
            <h2>{creating ? "新建题目" : `编辑 ${editing?.id}`}</h2>
            <button className="icon-button" onClick={() => { setCreating(false); setEditing(null); }} title="关闭"><X size={18} /></button>
          </div>
          <ProblemForm initial={editing ?? undefined} submitText={creating ? "创建题目" : "保存修改"} onSubmit={creating ? create : update} />
        </div>
      )}

      {items.length === 0 ? <div className="notice">当前没有题目</div> : (
        <div className="table-wrap">
          <table className="problem-table teacher-problem-table">
            <colgroup>
              <col className="col-id" />
              <col className="col-title" />
              <col className="col-short" />
              <col className="col-tags" />
              <col className="col-limit" />
              <col className="col-actions" />
            </colgroup>
            <thead><tr><th>题号</th><th>标题</th><th>难度</th><th>标签</th><th>限制</th><th></th></tr></thead>
            <tbody>
              {items.map((item) => (
                <tr key={item.id}>
                  <td className="mono">{item.id}</td>
                  <td className="strong-cell">{item.title}</td>
                  <td><StatusBadge difficulty={item.difficulty} /></td>
                  <td>{item.tags.map((tag) => <span className="tag" key={tag}>{tag}</span>)}</td>
                  <td>{item.time_limit}s / {item.memory_limit}MB</td>
                  <td className="row-actions">
                    <button className="icon-button" title="编辑" onClick={() => openEdit(item.id)}><Edit3 size={18} /></button>
                    <button className="icon-button danger" title="删除" onClick={() => setDeleteId(item.id)}><Trash2 size={18} /></button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
      {deleteId && (
        <ConfirmDialog
          title={`删除题目 ${deleteId}`}
          message="历史提交和日志会保留，题目配置会被删除"
          confirmText="确认删除"
          danger
          onConfirm={() => { void remove(deleteId); setDeleteId(null); }}
          onCancel={() => setDeleteId(null)}
        />
      )}
    </section>
  );
}
