import { useEffect, useState } from "react";
import { RefreshCw } from "lucide-react";
import { api, friendlyError, type ProblemListItem } from "../api/client";
import ErrorMessage from "../components/ErrorMessage";
import StatusBadge from "../components/StatusBadge";

type Props = {
  onOpen: (id: string) => void;
};

export default function ProblemsPage({ onOpen }: Props) {
  const [items, setItems] = useState<ProblemListItem[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  async function load() {
    setLoading(true);
    setError(null);
    try {
      setItems((await api.problems()).items);
    } catch (err) {
      setError(friendlyError(err, "题目列表加载失败，请稍后重试"));
    } finally {
      setLoading(false);
    }
  }

  useEffect(() => {
    void load();
  }, []);

  return (
    <section>
      <div className="page-head">
        <div>
          <h1>题目列表</h1>
        </div>
        <button onClick={load} title="刷新"><RefreshCw size={18} />刷新</button>
      </div>
      <ErrorMessage message={error} onClose={() => setError(null)} />
      {loading ? <div className="notice">加载中...</div> : items.length === 0 ? <div className="notice">当前没有题目</div> : (
        <div className="table-wrap">
          <table className="problem-table problem-list-table">
            <colgroup>
              <col className="col-id" />
              <col className="col-title" />
              <col className="col-short" />
              <col className="col-tags" />
              <col className="col-limit" />
            </colgroup>
            <thead>
              <tr><th>题号</th><th>标题</th><th>难度</th><th>标签</th><th>限制</th></tr>
            </thead>
            <tbody>
              {items.map((item) => (
                <tr key={item.id} className="clickable-row" onClick={() => onOpen(item.id)}>
                  <td className="mono">{item.id}</td>
                  <td className="strong-cell">{item.title}</td>
                  <td><StatusBadge difficulty={item.difficulty} /></td>
                  <td>{item.tags.map((tag) => <span className="tag" key={tag}>{tag}</span>)}</td>
                  <td>{item.time_limit}s / {item.memory_limit}MB</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </section>
  );
}
