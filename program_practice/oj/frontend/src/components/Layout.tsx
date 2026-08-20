import { Copy, DatabaseBackup, FileCode2, KeyRound, ListChecks, LogOut, Shield, UserCog } from "lucide-react";
import { useState } from "react";
import type { ReactNode } from "react";
import type { Role, User } from "../api/client";
import ConfirmDialog from "./ConfirmDialog";

export type View =
  | { name: "login" }
  | { name: "problems" }
  | { name: "problem"; id: string }
  | { name: "submissions" }
  | { name: "submission"; id: string }
  | { name: "teacher-problems" }
  | { name: "admin-users" }
  | { name: "admin-backups" };

type Props = {
  user: User | null;
  view: View;
  onNavigate: (view: View) => void;
  onLogout: () => void;
  children: ReactNode;
};

function canManageProblems(role: Role) {
  return role === "teacher" || role === "admin";
}

export default function Layout({ user, view, onNavigate, onLogout, children }: Props) {
  const [accountOpen, setAccountOpen] = useState(false);
  const [passwordNoticeOpen, setPasswordNoticeOpen] = useState(false);
  const [copyNotice, setCopyNotice] = useState("");
  const active = (name: View["name"]) => (view.name === name ? "active" : "");
  const problemActive = view.name === "problems" || view.name === "problem" ? "active" : "";
  const submissionActive = view.name === "submissions" || view.name === "submission" ? "active" : "";

  async function copyUserId() {
    if (!user) return;
    try {
      await navigator.clipboard.writeText(user.id);
      setCopyNotice("ID 已复制到剪贴板。");
    } catch {
      setCopyNotice("复制失败，请手动选择 ID 后复制。");
    }
  }

  return (
    <div className="shell">
      <header className="topbar">
        <button className="brand" onClick={() => onNavigate({ name: user ? "problems" : "login" })}>
          <FileCode2 size={24} />
          <span>OJ System</span>
        </button>
        <nav>
          <button className={problemActive} onClick={() => onNavigate({ name: "problems" })}>
            <ListChecks size={18} />
            <span>题目列表</span>
          </button>
          <button className={submissionActive} onClick={() => onNavigate({ name: "submissions" })}>
            <FileCode2 size={18} />
            <span>提交记录</span>
          </button>
          {user && canManageProblems(user.role) && (
            <button className={active("teacher-problems")} onClick={() => onNavigate({ name: "teacher-problems" })}>
              <Shield size={18} />
              <span>题目管理</span>
            </button>
          )}
          {user?.role === "admin" && (
            <>
              <button className={active("admin-users")} onClick={() => onNavigate({ name: "admin-users" })}>
                <UserCog size={18} />
                <span>用户管理</span>
              </button>
              <button className={active("admin-backups")} onClick={() => onNavigate({ name: "admin-backups" })}>
                <DatabaseBackup size={18} />
                <span>备份恢复</span>
              </button>
            </>
          )}
        </nav>
        <div className="account">
          {user ? (
            <div className="account-menu" onMouseEnter={() => setAccountOpen(true)} onMouseLeave={() => setAccountOpen(false)}>
              <button className="account-trigger" onClick={() => setAccountOpen((value) => !value)}>
                <strong>{user.username}</strong>
              </button>
              {accountOpen && (
                <div className="account-popover">
                  <div className="account-name">
                    <strong>{user.role}</strong>
                    <div className="account-id-row">
                      <span title={user.id}>ID: {user.id}</span>
                      <button type="button" className="icon-button account-id-copy" title="复制 ID" aria-label="复制 ID" onClick={copyUserId}>
                        <Copy size={14} />
                      </button>
                    </div>
                  </div>
                  <button type="button" onClick={() => { setAccountOpen(false); setPasswordNoticeOpen(true); }}>
                    <KeyRound size={18} />
                    <span>修改密码</span>
                  </button>
                  <button type="button" onClick={onLogout}>
                    <LogOut size={18} />
                    <span>登出</span>
                  </button>
                </div>
              )}
            </div>
          ) : (
            <button className="primary login-entry" onClick={() => onNavigate({ name: "login" })}>
              登录
            </button>
          )}
        </div>
      </header>
      <main className="content">{children}</main>
      {passwordNoticeOpen && (
        <ConfirmDialog
          title="修改密码"
          message="修改密码功能暂未实现。"
          confirmText="知道了"
          onConfirm={() => setPasswordNoticeOpen(false)}
        />
      )}
      {copyNotice && (
        <ConfirmDialog
          title={copyNotice.startsWith("id") ? "复制成功" : "操作失败"}
          message={copyNotice}
          confirmText="知道了"
          onConfirm={() => setCopyNotice("")}
        />
      )}
    </div>
  );
}
