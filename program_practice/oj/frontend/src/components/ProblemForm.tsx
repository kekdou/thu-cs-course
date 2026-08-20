import { useState } from "react";
import type { FormEvent } from "react";
import { Plus, Trash2 } from "lucide-react";
import type { Difficulty, Problem, Sample, TestCase } from "../api/client";
import ConfirmDialog from "./ConfirmDialog";
import ErrorMessage from "./ErrorMessage";

type Props = {
  initial?: Problem;
  onSubmit: (problem: Problem) => Promise<void>;
  submitText: string;
};

type DeleteTarget = {
  type: "sample" | "case";
  index: number;
  title: string;
} | null;

const emptyProblem: Problem = {
  id: "",
  title: "",
  description: "",
  input_description: "",
  output_description: "",
  samples: [{ input: "", output: "" }],
  constraints: "",
  time_limit: 1,
  memory_limit: 128,
  difficulty: "easy",
  tags: [],
  test_cases: [{ case_id: "case_01", input: "", output: "", score: 100, is_hidden: false }]
};

export default function ProblemForm({ initial, onSubmit, submitText }: Props) {
  const [problem, setProblem] = useState<Problem>(() => normalizeProblem(initial ?? emptyProblem));
  const [tagsText, setTagsText] = useState((initial?.tags ?? []).join(", "));
  const [deleteTarget, setDeleteTarget] = useState<DeleteTarget>(null);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const samples = problem.samples;
  const cases = problem.test_cases ?? [];
  const set = <K extends keyof Problem>(key: K, value: Problem[K]) => {
    setError(null);
    setProblem((current) => ({ ...current, [key]: value }));
  };

  async function submit(event: FormEvent) {
    event.preventDefault();
    const payload = cleanProblem(problem, tagsText);
    const message = validateProblem(payload, tagsText);
    if (message) {
      setError(message);
      return;
    }
    setBusy(true);
    setError(null);
    try {
      await onSubmit(payload);
      setError(null);
    } finally {
      setBusy(false);
    }
  }

  function cleanProblem(value: Problem, rawTags: string): Problem {
    return {
      ...value,
      tags: rawTags.split(",").map((tag) => tag.trim()).filter(Boolean),
      samples: value.samples.filter((sample) => sample.input.trim() || sample.output.trim()),
      test_cases: normalizeCases((value.test_cases ?? []).filter((item) => item.input.trim() || item.output.trim()))
    };
  }

  function normalizeProblem(value: Problem): Problem {
    return { ...value, test_cases: normalizeCases(value.test_cases ?? []) };
  }

  function validateProblem(value: Problem, rawTags: string): string | null {
    if (!/^[A-Za-z0-9_-]{1,32}$/.test(value.id)) return "题号只能包含字母、数字、下划线和连字符，长度 1 到 32";
    if (!value.title.trim()) return "标题不能为空";
    if (!value.description.trim() || !value.input_description.trim() || !value.output_description.trim()) return "题目描述、输入说明和输出说明都不能为空";
    if (!value.samples.length || value.samples.some((sample) => !sample.input.trim() || !sample.output.trim())) return "至少需要 1 组完整样例";
    if (!value.test_cases?.length) return "至少需要 1 个测试点";
    if (new Set(value.test_cases.map((item) => item.case_id.trim())).size !== value.test_cases.length) return "同一道题的测试点编号不能重复";
    if (Math.abs(value.test_cases.reduce((sum, item) => sum + Number(item.score), 0) - 100) > 0.01) return "所有测试点分值总和必须等于 100";
    if (rawTags.length > 200) return "标签内容过长，请精简后再提交";
    return null;
  }

  function scoresFor(count: number): number[] {
    if (count <= 0) return [];
    const base = Math.floor(100 / count);
    const remainder = 100 % count;
    return Array.from({ length: count }, (_, index) => base + (index < remainder ? 1 : 0));
  }

  function withEvenScores(nextCases: TestCase[]): TestCase[] {
    const scores = scoresFor(nextCases.length);
    return nextCases.map((item, index) => ({ ...item, case_id: caseId(index), score: scores[index] }));
  }

  function normalizeCases(nextCases: TestCase[]): TestCase[] {
    return nextCases.map((item, index) => ({ ...item, case_id: caseId(index) }));
  }

  function caseId(index: number): string {
    return `case_${String(index + 1).padStart(2, "0")}`;
  }

  function updateSample(index: number, patch: Partial<Sample>) {
    set("samples", samples.map((item, i) => (i === index ? { ...item, ...patch } : item)));
  }

  function updateCase(index: number, patch: Partial<TestCase>) {
    set("test_cases", cases.map((item, i) => (i === index ? { ...item, ...patch } : item)));
  }

  function addCase() {
    const next = [...cases, { case_id: caseId(cases.length), input: "", output: "", score: 0, is_hidden: true }];
    set("test_cases", withEvenScores(next));
  }

  function confirmDelete() {
    if (!deleteTarget) return;
    if (deleteTarget.type === "sample") {
      if (samples.length <= 1) {
        setError("至少需要保留 1 个样例编辑位");
      } else {
        set("samples", samples.filter((_, index) => index !== deleteTarget.index));
        setError(null);
      }
    } else if (cases.length <= 1) {
      setError("至少需要保留 1 个测试点编辑位");
    } else {
      set("test_cases", withEvenScores(cases.filter((_, index) => index !== deleteTarget.index)));
      setError(null);
    }
    setDeleteTarget(null);
  }

  return (
    <form className="form-grid" onSubmit={submit}>
      <label>
        题号
        <input value={problem.id} onChange={(event) => set("id", event.target.value)} disabled={Boolean(initial)} />
      </label>
      <label>
        标题
        <input value={problem.title} onChange={(event) => set("title", event.target.value)} />
      </label>
      <label>
        难度
        <select value={problem.difficulty} onChange={(event) => set("difficulty", event.target.value as Difficulty)}>
          <option value="easy">easy</option>
          <option value="medium">medium</option>
          <option value="hard">hard</option>
        </select>
      </label>
      <label>
        标签
        <input value={tagsText} onChange={(event) => setTagsText(event.target.value)} placeholder="基础, 数学" />
      </label>
      <label>
        时间限制
        <input type="number" min="0.1" step="0.1" value={problem.time_limit} onChange={(event) => set("time_limit", Number(event.target.value))} />
      </label>
      <label>
        内存限制
        <input type="number" min="1" value={problem.memory_limit} onChange={(event) => set("memory_limit", Number(event.target.value))} />
      </label>
      <label className="span-2">
        题目描述
        <textarea value={problem.description} onChange={(event) => set("description", event.target.value)} />
      </label>
      <label>
        输入说明
        <textarea value={problem.input_description} onChange={(event) => set("input_description", event.target.value)} />
      </label>
      <label>
        输出说明
        <textarea value={problem.output_description} onChange={(event) => set("output_description", event.target.value)} />
      </label>
      <label className="span-2">
        约束
        <input value={problem.constraints} onChange={(event) => set("constraints", event.target.value)} />
      </label>

      <section className="span-2 inline-section">
        <div className="section-head">
          <h3>样例</h3>
          <button type="button" onClick={() => set("samples", [...samples, { input: "", output: "" }])}>
            <Plus size={17} />
            新增样例
          </button>
        </div>
        <div className="editor-list">
          {samples.map((sample, index) => (
            <article className="editor-item" key={index}>
              <header>
                <strong>样例 {index + 1}</strong>
                <button type="button" className="icon-button danger" title="删除样例" onClick={() => setDeleteTarget({ type: "sample", index, title: `样例 ${index + 1}` })}>
                  <Trash2 size={18} />
                </button>
              </header>
              <div className="case-grid">
                <label>
                  输入样例
                  <textarea value={sample.input} onChange={(event) => updateSample(index, { input: event.target.value })} />
                </label>
                <label>
                  输出样例
                  <textarea value={sample.output} onChange={(event) => updateSample(index, { output: event.target.value })} />
                </label>
              </div>
            </article>
          ))}
        </div>
      </section>

      <section className="span-2 inline-section">
        <div className="section-head">
          <h3>测试点</h3>
          <button type="button" onClick={addCase}>
            <Plus size={17} />
            新增测试点
          </button>
        </div>
        <div className="editor-list">
          {cases.map((testCase, index) => (
            <article className="editor-item" key={index}>
              <header>
                <strong>{caseId(index)}</strong>
                <button type="button" className="icon-button danger" title="删除测试点" onClick={() => setDeleteTarget({ type: "case", index, title: testCase.case_id || `测试点 ${index + 1}` })}>
                  <Trash2 size={18} />
                </button>
              </header>
              <div className="testcase-editor">
                <label>
                  分值
                  <input type="number" min="0" step="0.01" value={testCase.score} onChange={(event) => updateCase(index, { score: Number(event.target.value) })} />
                </label>
                <label className="check">
                  <input type="checkbox" checked={testCase.is_hidden} onChange={(event) => updateCase(index, { is_hidden: event.target.checked })} />
                  隐藏
                </label>
                <label>
                  输入
                  <textarea value={testCase.input} onChange={(event) => updateCase(index, { input: event.target.value })} />
                </label>
                <label>
                  输出
                  <textarea value={testCase.output} onChange={(event) => updateCase(index, { output: event.target.value })} />
                </label>
              </div>
            </article>
          ))}
        </div>
      </section>

      <ErrorMessage message={error} onClose={() => setError(null)} />
      <button className="primary span-2" disabled={busy}>{busy ? "提交中..." : submitText}</button>

      {deleteTarget && (
        <ConfirmDialog
          title={`删除${deleteTarget.title}`}
          message="删除后该条目会从当前题目配置中移除，保存前仍可取消整个编辑"
          confirmText="确认删除"
          danger
          onConfirm={confirmDelete}
          onCancel={() => setDeleteTarget(null)}
        />
      )}
    </form>
  );
}
