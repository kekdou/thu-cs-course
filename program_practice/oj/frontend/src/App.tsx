import { useEffect, useState } from "react";
import { api, friendlyError, type User } from "./api/client";
import Layout, { type View } from "./components/Layout";
import ErrorMessage from "./components/ErrorMessage";
import LoginPage from "./pages/LoginPage";
import ProblemsPage from "./pages/ProblemsPage";
import ProblemDetailPage from "./pages/ProblemDetailPage";
import SubmissionsPage from "./pages/SubmissionsPage";
import SubmissionDetailPage from "./pages/SubmissionDetailPage";
import TeacherProblemsPage from "./pages/TeacherProblemsPage";
import AdminUsersPage from "./pages/AdminUsersPage";
import AdminBackupsPage from "./pages/AdminBackupsPage";

export default function App() {
  const [user, setUser] = useState<User | null>(null);
  const [view, setView] = useState<View>({ name: "login" });
  const [history, setHistory] = useState<View[]>([]);
  const [booting, setBooting] = useState(true);
  const [notice, setNotice] = useState<string | null>(null);
  const [drafts, setDrafts] = useState<Record<string, string>>({});

  useEffect(() => {
    api.me()
      .then((current) => {
        setUser(current);
        setView({ name: "problems" });
      })
      .catch(() => setView({ name: "login" }))
      .finally(() => setBooting(false));
  }, []);

  async function logout() {
    setNotice(null);
    try {
      await api.logout();
    } catch (err) {
      setNotice(friendlyError(err, "登出失败，已在本地清理登录状态。"));
    } finally {
      setUser(null);
      setView({ name: "login" });
      setHistory([]);
    }
  }

  function requireLogin(next: View) {
    if (!user && next.name !== "login") {
      setNotice("请先登录");
      setView({ name: "login" });
      setHistory([]);
      return;
    }
    setNotice(null);
    setView(next);
    setHistory([]);
  }

  function navigate(next: View) {
    setNotice(null);
    setHistory((current) => [...current, view]);
    setView(next);
  }

  function goBack(fallback: View) {
    const previous = history[history.length - 1];
    if (!previous) {
      setView(fallback);
      return;
    }
    setHistory((current) => current.slice(0, -1));
    setView(previous);
  }

  function renderView() {
    if (booting) return <div className="notice">正在连接后端...</div>;
    if (!user || view.name === "login") {
      return <LoginPage onLoggedIn={(current) => { setUser(current); setNotice(null); setView({ name: "problems" }); }} />;
    }
    if (view.name === "problems") return <ProblemsPage onOpen={(id) => navigate({ name: "problem", id })} />;
    if (view.name === "problem") {
      return (
        <ProblemDetailPage
          problemId={view.id}
          code={drafts[view.id] ?? ""}
          onCodeChange={(code) => setDrafts((current) => ({ ...current, [view.id]: code }))}
          onBack={() => goBack({ name: "problems" })}
          onSubmissionCreated={(id) => navigate({ name: "submission", id })}
        />
      );
    }
    if (view.name === "submissions") return <SubmissionsPage user={user} onOpen={(id) => navigate({ name: "submission", id })} />;
    if (view.name === "submission") return <SubmissionDetailPage submissionId={view.id} user={user} onBack={() => goBack({ name: "submissions" })} />;
    if (view.name === "teacher-problems" && (user.role === "teacher" || user.role === "admin")) return <TeacherProblemsPage />;
    if (view.name === "admin-users" && user.role === "admin") return <AdminUsersPage />;
    if (view.name === "admin-backups" && user.role === "admin") return <AdminBackupsPage />;
    return <ErrorMessage message="无权限访问当前页面" />;
  }

  return (
    <Layout user={user} view={view} onNavigate={requireLogin} onLogout={logout}>
      <ErrorMessage message={notice} onClose={() => setNotice(null)} />
      {renderView()}
    </Layout>
  );
}
