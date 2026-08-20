import type { JudgeResult, SubmissionStatus } from "../api/client";

type Props = {
  status?: SubmissionStatus | null;
  result?: JudgeResult | null;
  difficulty?: string | null;
};

export default function StatusBadge({ status, result, difficulty }: Props) {
  const value = result ?? status ?? difficulty ?? "unknown";
  return <span className={`badge badge-${value.toLowerCase()}`}>{value}</span>;
}
