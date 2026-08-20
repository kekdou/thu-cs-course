export type Role = "student" | "teacher" | "admin";
export type Difficulty = "easy" | "medium" | "hard";
export type SubmissionStatus = "pending" | "running" | "finished" | "failed";
export type JudgeResult = "AC" | "WA" | "RE" | "TLE" | "MLE" | "SE";

export type Page<T> = {
  items: T[];
  total: number;
  page: number;
  page_size: number;
};

export type User = {
  id: string;
  username: string;
  role: Role;
  is_active: boolean;
  created_at: string;
  updated_at: string;
};

export type Sample = {
  input: string;
  output: string;
};

export type TestCase = {
  case_id: string;
  input: string;
  output: string;
  score: number;
  is_hidden: boolean;
};

export type Problem = {
  id: string;
  title: string;
  description: string;
  input_description: string;
  output_description: string;
  samples: Sample[];
  constraints: string;
  time_limit: number;
  memory_limit: number;
  difficulty: Difficulty;
  tags: string[];
  test_cases?: TestCase[];
  created_at?: string;
  updated_at?: string;
};

export type ProblemListItem = Pick<Problem, "id" | "title" | "difficulty" | "tags" | "time_limit" | "memory_limit">;

export type Submission = {
  id: string;
  submission_id?: string;
  user_id: string;
  username?: string | null;
  problem_id: string;
  language: "python";
  source_code?: string;
  status: SubmissionStatus;
  result: JudgeResult | null;
  score: number;
  total_time: number | null;
  created_at: string;
  started_at: string | null;
  finished_at: string | null;
};

export type CaseLog = {
  submission_id?: string;
  problem_id?: string;
  user_id?: string;
  case_id: string;
  result: JudgeResult;
  score: number;
  time_used: number;
  memory_used: number | null;
  exit_code?: number | null;
  stdout?: string | null;
  stderr: string;
  message: string;
  is_hidden: boolean;
  input_data?: string;
  expected_output?: string;
  created_at: string;
};

export type Backup = {
  backup_id: string;
  created_at: string;
  path?: string | null;
  manifest?: Record<string, unknown> | null;
};

export type SubmissionFilters = {
  problem_id?: string;
  user_id?: string;
  status?: SubmissionStatus;
  result?: JudgeResult;
};

type ApiEnvelope<T> = {
  code: number;
  message: string;
  data: T;
};

export class ApiError extends Error {
  code: number;

  constructor(code: number, message: string) {
    super(message);
    this.code = code;
  }
}

export function friendlyError(err: unknown, fallback: string): string {
  if (!(err instanceof ApiError)) return fallback;
  const detail = err.message.toLowerCase();
  if (err.code === 0) return "网络连接失败，请确认后端服务正在运行后重试";
  if (err.code === 401) return "用户名或密码不正确，或登录已失效，请重新登录";
  if (err.code === 403 && detail.includes("disabled")) return "当前账号已被禁用，无法继续操作";
  if (err.code === 403) return "当前账号没有执行此操作的权限";
  if (err.code === 404 && detail.includes("problem")) return "题目不存在，请刷新题目列表后再试";
  if (err.code === 404 && detail.includes("submission")) return "提交记录不存在，请刷新提交列表后再试";
  if (err.code === 404 && detail.includes("backup")) return "备份不存在，请刷新备份列表后再试";
  if (err.code === 404) return "请求的资源不存在，请刷新列表后再试";
  if (err.code === 409 && detail.includes("username")) return "用户名已被注册，请换一个用户名或直接登录";
  if (err.code === 409 && detail.includes("problem")) return "题号已存在，请更换题号后再创建";
  if (err.code === 409 && detail.includes("judged")) return "该提交正在评测中，完成后才能重新评测";
  if (err.code === 409) return "数据已存在或当前状态不允许操作，请检查后重试";
  if (err.code === 422) return err.message || "输入内容格式不符合要求，请检查必填项、长度和数值范围";
  if (err.code >= 500 && detail.includes("problem save failed")) return "题目保存失败，请稍后重试或检查数据库写入权限";
  if (err.code >= 500) return "后端服务暂时异常，请稍后重试";
  return err.message || fallback;
}

const API_BASE = import.meta.env.VITE_API_BASE ?? "";

async function request<T>(path: string, init: RequestInit = {}): Promise<T> {
  let response: Response;
  try {
    response = await fetch(`${API_BASE}${path}`, {
      ...init,
      credentials: "include",
      headers: {
        ...(init.body ? { "Content-Type": "application/json" } : {}),
        ...init.headers
      }
    });
  } catch {
    throw new ApiError(0, "网络请求失败，请确认后端服务已启动");
  }

  let body: ApiEnvelope<T | null>;
  try {
    body = await response.json();
  } catch {
    throw new ApiError(response.status, "后端响应格式错误");
  }

  if (!response.ok) {
    throw new ApiError(body.code || response.status, body.message || "请求失败");
  }
  return body.data as T;
}

function qs(params: Record<string, string | number | undefined | null>): string {
  const query = new URLSearchParams();
  Object.entries(params).forEach(([key, value]) => {
    if (value !== undefined && value !== null && value !== "") query.set(key, String(value));
  });
  const text = query.toString();
  return text ? `?${text}` : "";
}

export const api = {
  register: (username: string, password: string) =>
    request<User>("/api/auth/register", { method: "POST", body: JSON.stringify({ username, password }) }),
  login: (username: string, password: string) =>
    request<User>("/api/auth/login", { method: "POST", body: JSON.stringify({ username, password }) }),
  logout: () => request<null>("/api/auth/logout", { method: "POST" }),
  me: () => request<User>("/api/auth/me"),

  problems: (page = 1, pageSize = 20) => request<Page<ProblemListItem>>(`/api/problems${qs({ page, page_size: pageSize })}`),
  problem: (id: string) => request<Problem>(`/api/problems/${encodeURIComponent(id)}`),
  createProblem: (problem: Problem) => request<Problem>("/api/problems", { method: "POST", body: JSON.stringify(problem) }),
  updateProblem: (id: string, problem: Problem) =>
    request<Problem>(`/api/problems/${encodeURIComponent(id)}`, { method: "PUT", body: JSON.stringify(problem) }),
  deleteProblem: (id: string) => request<null>(`/api/problems/${encodeURIComponent(id)}`, { method: "DELETE" }),

  createSubmission: (problemId: string, sourceCode: string) =>
    request<Submission>("/api/submissions", {
      method: "POST",
      body: JSON.stringify({ problem_id: problemId, language: "python", source_code: sourceCode })
    }),
  submissions: (page = 1, pageSize = 20, filters: SubmissionFilters = {}) =>
    request<Page<Submission>>(`/api/submissions${qs({ page, page_size: pageSize, ...filters })}`),
  submission: (id: string) => request<Submission>(`/api/submissions/${encodeURIComponent(id)}`),
  submissionLogs: (id: string) => request<CaseLog[]>(`/api/submissions/${encodeURIComponent(id)}/logs`),
  rejudgeSubmission: (id: string) =>
    request<Submission>(`/api/submissions/${encodeURIComponent(id)}/rejudge`, { method: "POST" }),

  users: (page = 1, pageSize = 50) => request<Page<User>>(`/api/users${qs({ page, page_size: pageSize })}`),
  updateUser: (id: string, role: Role, isActive: boolean) =>
    request<User>(`/api/users/${encodeURIComponent(id)}`, { method: "PUT", body: JSON.stringify({ role, is_active: isActive }) }),

  backups: () => request<Backup[]>("/api/admin/backups"),
  createBackup: () => request<Backup>("/api/admin/backups", { method: "POST" }),
  restoreBackup: (id: string) => request<Backup>(`/api/admin/backups/${encodeURIComponent(id)}/restore`, { method: "POST" })
};
