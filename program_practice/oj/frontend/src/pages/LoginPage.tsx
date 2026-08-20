import { useState } from "react";
import type { FormEvent } from "react";
import { Eye, EyeOff, LogIn, UserPlus } from "lucide-react";
import { api, friendlyError, type User } from "../api/client";
import ErrorMessage from "../components/ErrorMessage";

type Props = {
  onLoggedIn: (user: User) => void;
};

export default function LoginPage({ onLoggedIn }: Props) {
  const [mode, setMode] = useState<"login" | "register">("login");
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [error, setError] = useState<string | null>(null);
  const [fieldError, setFieldError] = useState<{ username?: string; password?: string }>({});
  const [busy, setBusy] = useState(false);
  const [showPassword, setShowPassword] = useState(false);

  async function submit(event: FormEvent) {
    event.preventDefault();
    const nextFieldError = validateLoginForm(username, password);
    setFieldError(nextFieldError);
    if (nextFieldError.username || nextFieldError.password) return;

    setBusy(true);
    setError(null);
    try {
      const user = mode === "login" ? await api.login(username, password) : await registerAndLogin(username, password);
      onLoggedIn(user);
    } catch (err) {
      setError(friendlyError(err, mode === "login" ? "登录失败，请检查用户名和密码" : "注册失败，请检查输入内容"));
    } finally {
      setBusy(false);
    }
  }

  async function registerAndLogin(name: string, secret: string) {
    await api.register(name, secret);
    return api.login(name, secret);
  }

  function validateLoginForm(name: string, secret: string) {
    const next: { username?: string; password?: string } = {};
    if (name.trim().length < 3 || name.trim().length > 32) next.username = "用户名需要 3 到 32 个字符";
    if (secret.length < 8) next.password = "密码至少需要 8 个字符";
    return next;
  }

  return (
    <section className="auth-panel">
      <div className="segmented">
        <button className={mode === "login" ? "active" : ""} onClick={() => setMode("login")}>登录</button>
        <button className={mode === "register" ? "active" : ""} onClick={() => setMode("register")}>注册</button>
      </div>
      <form className="stack" onSubmit={submit}>
        <label>
          用户名
          <input value={username} onChange={(event) => { setUsername(event.target.value); setFieldError((current) => ({ ...current, username: undefined })); }} autoComplete="username" aria-invalid={Boolean(fieldError.username)} />
          {fieldError.username && <span className="field-error">{fieldError.username}</span>}
        </label>
        <label>
          密码
          <span className="password-field">
            <input type={showPassword ? "text" : "password"} value={password} onChange={(event) => { setPassword(event.target.value); setFieldError((current) => ({ ...current, password: undefined })); }} autoComplete={mode === "login" ? "current-password" : "new-password"} aria-invalid={Boolean(fieldError.password)} />
            <button type="button" className="password-toggle" onClick={() => setShowPassword((value) => !value)} title={showPassword ? "隐藏密码" : "显示密码"}>
              {showPassword ? <EyeOff size={18} /> : <Eye size={18} />}
            </button>
          </span>
          {fieldError.password && <span className="field-error">{fieldError.password}</span>}
        </label>
        <ErrorMessage message={error} onClose={() => setError(null)} />
        <button className="primary" disabled={busy}>
          {mode === "login" ? <LogIn size={18} /> : <UserPlus size={18} />}
          {busy ? "处理中..." : mode === "login" ? "登录" : "注册并登录"}
        </button>
      </form>
    </section>
  );
}
