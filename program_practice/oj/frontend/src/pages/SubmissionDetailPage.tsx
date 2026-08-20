import { useEffect, useState } from "react";
import { ArrowLeft, RefreshCw, RotateCcw } from "lucide-react";
import { api, friendlyError, type CaseLog, type Submission, type User } from "../api/client";
import ConfirmDialog from "../components/ConfirmDialog";
import ErrorMessage from "../components/ErrorMessage";
import StatusBadge from "../components/StatusBadge";

type Props = {
  submissionId: string;
  user: User;
  onBack: () => void;
};

export default function SubmissionDetailPage({ submissionId, user, onBack }: Props) {
  const [submission, setSubmission] = useState<Submission | null>(null);
  const [logs, setLogs] = useState<CaseLog[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [message, setMessage] = useState<string | null>(null);
  const [loading, setLoading] = useState(true);
  const [rejudgeOpen, setRejudgeOpen] = useState(false);
  const [rejudging, setRejudging] = useState(false);

  const canRejudge = Boolean(
    submission &&
    (user.role === "teacher" || user.role === "admin") &&
    (submission.status === "finished" || submission.status === "failed")
  );
  const canViewFullLogs = user.role === "teacher" || user.role === "admin";

  async function load() {
    setLoading(true);
    setError(null);
    try {
      const [nextSubmission, nextLogs] = await Promise.all([api.submission(submissionId), api.submissionLogs(submissionId)]);
      setSubmission(nextSubmission);
      setLogs(nextLogs);
    } catch (err) {
      setError(friendlyError(err, "提交详情加载失败，请刷新后重试"));
    } finally {
      setLoading(false);
    }
  }

  async function rejudge() {
    setRejudging(true);
    setError(null);
    setMessage(null);
    try {
      const nextSubmission = await api.rejudgeSubmission(submissionId);
      setSubmission(nextSubmission);
      setLogs([]);
      setMessage("重新评测任务已提交");
      await load();
    } catch (err) {
      setError(friendlyError(err, "重新评测失败，请确认提交已完成且你有教师或管理员权限"));
    } finally {
      setRejudging(false);
      setRejudgeOpen(false);
    }
  }

  useEffect(() => {
    void load();
    const timer = window.setInterval(() => {
      if (submission?.status === "pending" || submission?.status === "running") void load();
    }, 1500);
    return () => window.clearInterval(timer);
  }, [submissionId, submission?.status]);

  return (
    <section>
      <div className="page-head">
        <div>
          <button className="back-button" onClick={onBack}><ArrowLeft size={18} />返回</button>
          <h1>提交详情</h1>
          <p className="mono">{submissionId}</p>
        </div>
        <div className="actions">
          {canRejudge && (
            <button onClick={() => setRejudgeOpen(true)} disabled={rejudging} title="重新评测">
              <RotateCcw size={18} />{rejudging ? "提交中..." : "重新评测"}
            </button>
          )}
          <button onClick={load} title="刷新"><RefreshCw size={18} />刷新</button>
        </div>
      </div>
      <ErrorMessage message={error} onClose={() => setError(null)} />
      {message && <div className="notice success">{message}</div>}
      {loading && <div className="notice">加载中...</div>}
      {submission && (
        <>
          <div className="summary-grid">
            <div><span>题目</span><strong>{submission.problem_id}</strong></div>
            <div><span>状态</span><StatusBadge status={submission.status} /></div>
            <div><span>结果</span>{submission.result ? <StatusBadge result={submission.result} /> : <strong>-</strong>}</div>
            <div><span>得分</span><strong>{submission.score}</strong></div>
            <div><span>总时间</span><strong>{submission.total_time ?? "-"}s</strong></div>
          </div>
          {submission.source_code && <pre className="source-view">{submission.source_code}</pre>}
        </>
      )}
      <h2>测试点日志</h2>
      {logs.length === 0 ? (
        <div className="notice">
          {submission?.status === "pending" || submission?.status === "running" ? "暂无日志，评测仍在进行" : "暂无可展示日志，评测系统可能发生异常"}
        </div>
      ) : logs.map((log) => (
        <article className="log-item" key={log.case_id}>
          <header>
            <strong>{log.case_id}</strong>
            <div className="log-header-tags">
              {canViewFullLogs && <span>{log.is_hidden ? "hidden" : "public"}</span>}
              <StatusBadge result={log.result} />
            </div>
          </header>
          <div className="log-meta">
            score {log.score} · time {log.time_used}s{log.exit_code !== undefined ? ` · exit ${log.exit_code ?? "-"}` : ""} · {log.created_at}
          </div>
          {canViewFullLogs && log.input_data !== undefined && <pre>input: {log.input_data}</pre>}
          {log.stdout && <pre>stdout: {log.stdout}</pre>}
          {log.expected_output && <pre>expected: {log.expected_output}</pre>}
          {log.stderr && <pre>stderr: {log.stderr}</pre>}
          {log.message && <pre>message: {log.message}</pre>}
        </article>
      ))}
      {rejudgeOpen && (
        <ConfirmDialog
          title="重新评测"
          message="确认重新评测该提交吗？当前测试点日志会被清空，并由后台生成新的评测结果"
          confirmText="确认评测"
          onConfirm={() => { void rejudge(); }}
          onCancel={() => setRejudgeOpen(false)}
        />
      )}
    </section>
  );
}
