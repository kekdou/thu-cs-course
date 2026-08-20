type Props = {
  value: string;
  onChange: (value: string) => void;
};

export default function CodeEditor({ value, onChange }: Props) {
  function handleKeyDown(event: KeyboardEvent<HTMLTextAreaElement>) {
    if (event.key !== "Tab") return;
    event.preventDefault();
    const target = event.currentTarget;
    const start = target.selectionStart;
    const end = target.selectionEnd;
    const nextValue = value.slice(0, start) + "    " + value.slice(end);
    onChange(nextValue);
    window.requestAnimationFrame(() => {
      target.selectionStart = start + 4;
      target.selectionEnd = start + 4;
    });
  }

  return (
    <textarea
      className="code-editor"
      spellCheck={false}
      value={value}
      onChange={(event) => onChange(event.target.value)}
      onKeyDown={handleKeyDown}
      placeholder="your code..."
    />
  );
}
import type { KeyboardEvent } from "react";
