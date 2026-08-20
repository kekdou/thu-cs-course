import { createPortal } from "react-dom";
import { X } from "lucide-react";

type Props = {
  title: string;
  message: string;
  confirmText?: string;
  cancelText?: string;
  danger?: boolean;
  onConfirm: () => void;
  onCancel?: () => void;
};

export default function ConfirmDialog({
  title,
  message,
  confirmText = "确认",
  cancelText = "取消",
  danger = false,
  onConfirm,
  onCancel
}: Props) {
  return createPortal(
    <div className="modal-backdrop" role="dialog" aria-modal="true">
      <div className="confirm-dialog">
        {onCancel && (
          <button type="button" className="icon-button dialog-close" onClick={onCancel} title="关闭">
            <X size={18} />
          </button>
        )}
        <h3>{title}</h3>
        <p>{message}</p>
        <div className="dialog-actions">
          <button type="button" className={danger ? "danger-button" : "primary"} onClick={onConfirm}>
            {confirmText}
          </button>
          {onCancel && <button type="button" onClick={onCancel}>{cancelText}</button>}
        </div>
      </div>
    </div>,
    document.body
  );
}
