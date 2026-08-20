import { useEffect, useState } from "react";
import ConfirmDialog from "./ConfirmDialog";

type Props = {
  message: string | null;
  onClose?: () => void;
};

export default function ErrorMessage({ message, onClose }: Props) {
  const [visible, setVisible] = useState(Boolean(message));

  useEffect(() => {
    setVisible(Boolean(message));
  }, [message]);

  if (!message || !visible) return null;
  return (
    <ConfirmDialog
      title="操作失败"
      message={message}
      confirmText="确认"
      danger
      onConfirm={() => {
        setVisible(false);
        onClose?.();
      }}
    />
  );
}
