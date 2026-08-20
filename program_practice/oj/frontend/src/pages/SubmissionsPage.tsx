import { useEffect, useState } from "react";
import type { FormEvent } from "react";
import { Filter, RefreshCw, X } from "lucide-react";
import { api, friendlyError, type JudgeResult, type Submission, type SubmissionFilters, type SubmissionStatus, type User } from "../api/client";
import ErrorMessage from "../components/ErrorMessage";
import StatusBadge from "../components/StatusBadge";

type Props = {
  user: User;
  onOpen: (id: string) => void;
};

const emptyFilters: SubmissionFilters = {
  problem_id: "",
  user_id: "",
  status: undefined,
  result: undefined
};

export default function SubmissionsPage({ user, onOpen }: Props) {
  const [items, setItems] = useState<Submission[]>([]);
  const [filters, setFilters] = useState<SubmissionFilters>(emptyFilters);
  const [appliedFilters, setAppliedFilters] = useState<SubmissionFilters>(emptyFilters);
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(true);
  const canFilterAll = user.role === "teacher" || user.role === "admin";

  async function load(nextFilters = appliedFilters) {
    setLoading(true);
    setError(null);
    try {
      setItems((await api.submissions(1, 20, cleanFilters(nextFilters))).items);
    } catch (err) {
      setError(friendlyError(err, "提交记录加载失败，请稍后重试"));
    } finally {
      setLoading(false);
    }
  }

  function submitFilters(event: FormEvent) {
    event.preventDefault();
    setAppliedFilters(filters);
    void load(filters);
  }

  function resetFilters() {
    setFilters(emptyFilters);
    setAppliedFilters(emptyFilters);
    void load(emptyFilters);
  }

  function cleanFilters(value: SubmissionFilters): SubmissionFilters {
    return Object.fromEntries(Object.entries(value).filter(([, item]) => item)) as SubmissionFilters;
  }

  useEffect(() => {
    void load();
  }, []);

  return (
    <section>
      <div className="page-head">
        <div><h1>提交记录</h1></div>
        <button onClick={() => void load()} title="刷新"><RefreshCw size={18} />刷新</button>
      </div>
      <form className="filter-bar" onSubmit={submitFilters}>
        <label>
          题号
          <input value={filters.problem_id ?? ""} onChange={(event) => setFilters((current) => ({ ...current, problem_id: event.target.value }))} placeholder="P1001" />
        </label>
        {canFilterAll && (
          <label>
            用户ID
            <input value={filters.user_id ?? ""} onChange={(event) => setFilters((current) => ({ ...current, user_id: event.target.value }))} placeholder="用户 id" />
          </label>
        )}
        <label>
          状态
          <select value={filters.status ?? ""} onChange={(event) => setFilters((current) => ({ ...current, status: event.target.value as SubmissionStatus || undefined }))}>
            <option value="">全部</option>
            <option value="pending">pending</option>
            <option value="running">running</option>
            <option value="finished">finished</option>
            <option value="failed">failed</option>
          </select>
        </label>
        <label>
          结果
          <select value={filters.result ?? ""} onChange={(event) => setFilters((current) => ({ ...current, result: event.target.value as JudgeResult || undefined }))}>
            <option value="">全部</option>
            <option value="AC">AC</option>
            <option value="WA">WA</option>
            <option value="RE">RE</option>
            <option value="TLE">TLE</option>
            <option value="MLE">MLE</option>
            <option value="SE">SE</option>
          </select>
        </label>
        <div className="filter-actions">
          <button className="primary" type="submit"><Filter size={18} />筛选</button>
          <button type="button" onClick={resetFilters}><X size={18} />清空</button>
        </div>
      </form>
      <ErrorMessage message={error} onClose={() => setError(null)} />
      {loading ? <div className="notice">加载中...</div> : items.length === 0 ? <div className="notice">暂无提交</div> : (
        <div className="table-wrap">
          <table className="submission-table">
            <colgroup>
              <col className="col-time" />
              <col className="col-user" />
              <col className="col-id" />
              <col className="col-short" />
              <col className="col-short" />
              <col className="col-score" />
            </colgroup>
            <thead><tr><th>时间</th><th>用户</th><th>题目</th><th>状态</th><th>结果</th><th>得分</th></tr></thead>
            <tbody>
              {items.map((item) => (
                <tr key={item.id} className="clickable-row" onClick={() => onOpen(item.id)}>
                  <td>{new Date(item.created_at).toLocaleString()}</td>
                  <td className="strong-cell">{item.username ?? item.user_id}</td>
                  <td className="mono">{item.problem_id}</td>
                  <td><StatusBadge status={item.status} /></td>
                  <td>{item.result ? <StatusBadge result={item.result} /> : "-"}</td>
                  <td>{item.score}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </section>
  );
}
