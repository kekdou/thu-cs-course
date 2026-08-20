import { useEffect, useState } from "react";
import { ArrowLeft, Send } from "lucide-react";
import { api, friendlyError, type Problem } from "../api/client";
import CodeEditor from "../components/CodeEditor";
import ErrorMessage from "../components/ErrorMessage";
import StatusBadge from "../components/StatusBadge";

type Props = {
  problemId: string;
  code: string;
  onCodeChange: (code: string) => void;
  onSubmissionCreated: (id: string) => void;
  onBack: () => void;
};

export default function ProblemDetailPage({ problemId, code, onCodeChange, onSubmissionCreated, onBack }: Props) {
  const [problem, setProblem] = useState<Problem | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [message, setMessage] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    api.problem(problemId).then(setProblem).catch((err) => setError(friendlyError(err, "题目加载失败，请刷新后重试")));
  }, [problemId]);

  async function submit() {
    setBusy(true);
    setError(null);
    setMessage(null);
    if (!code.trim()) {
      setError("源码不能为空，请输入 Python 代码后再提交");
      setBusy(false);
      return;
    }
    try {
      const submission = await api.createSubmission(problemId, code);
      setMessage(`提交已创建：${submission.id}`);
      onSubmissionCreated(submission.id);
    } catch (err) {
      setError(friendlyError(err, "提交失败，请检查源码和登录状态"));
    } finally {
      setBusy(false);
    }
  }

  if (!problem) return <><ErrorMessage message={error} onClose={() => setError(null)} /><div className="notice">加载题目中...</div></>;

  return (
    <section className="split-view">
      <article className="problem-body">
        <div className="page-head compact">
          <div>
            <button className="back-button" onClick={onBack}><ArrowLeft size={18} />返回</button>
            <h1>{problem.id} {problem.title}</h1>
            <p>{problem.time_limit}s / {problem.memory_limit}MB</p>
          </div>
          <StatusBadge difficulty={problem.difficulty} />
        </div>
        <p className="preline">{problem.description}</p>
        <h2>输入说明</h2>
        <p className="preline">{problem.input_description}</p>
        <h2>输出说明</h2>
        <p className="preline">{problem.output_description}</p>
        {problem.constraints && <><h2>约束</h2><p>{problem.constraints}</p></>}
        <h2>样例</h2>
        {problem.samples.map((sample, index) => (
          <div className="sample-pair" key={index}>
            <h3>样例 {index + 1}</h3>
            <div>
              <span>输入样例</span>
              <pre>{sample.input}</pre>
            </div>
            <div>
              <span>输出样例</span>
              <pre>{sample.output}</pre>
            </div>
          </div>
        ))}
        {problem.test_cases && (
          <>
            <h2>测试点配置</h2>
            <div className="table-wrap">
              <table className="case-config-table">
                <thead>
                  <tr><th>编号</th><th>分值</th><th>隐藏</th><th>输入</th><th>标准输出</th></tr>
                </thead>
                <tbody>
                  {problem.test_cases.map((testCase) => (
                    <tr key={testCase.case_id}>
                      <td className="mono">{testCase.case_id}</td>
                      <td>{testCase.score}</td>
                      <td>{testCase.is_hidden ? "是" : "否"}</td>
                      <td><pre>{testCase.input}</pre></td>
                      <td><pre>{testCase.output}</pre></td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
            {(problem.created_at || problem.updated_at) && (
              <p className="config-meta">
                创建：{problem.created_at ? new Date(problem.created_at).toLocaleString() : "-"} / 更新：{problem.updated_at ? new Date(problem.updated_at).toLocaleString() : "-"}
              </p>
            )}
          </>
        )}
      </article>
      <aside className="submit-panel">
        <h2>提交代码</h2>
        <CodeEditor value={code} onChange={onCodeChange} />
        <ErrorMessage message={error} onClose={() => setError(null)} />
        {message && <div className="notice success">{message}</div>}
        <button className="primary wide" onClick={submit} disabled={busy}>
          <Send size={18} />
          {busy ? "提交中..." : "提交"}
        </button>
      </aside>
    </section>
  );
}
