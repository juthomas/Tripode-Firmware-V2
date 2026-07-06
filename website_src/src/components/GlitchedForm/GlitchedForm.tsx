import { FormEvent, useEffect, useState } from "react";
import { GlitchedButton } from "../GlitchedButton";
import { GlitchedInput } from "../GlitchedInput";

interface GlitchedFormProps {
  label?: string;
  placeholder?: string;
  value?: string;
  onSubmit?: (value: string) => void;
}

export const GlitchedForm = ({
  label = "default",
  placeholder = label,
  value,
  onSubmit,
}: GlitchedFormProps): JSX.Element => {
  const [inputState, setInputState] = useState(value || "");

  useEffect(() => {
    if (value !== undefined) setInputState(value);
  }, [value]);

  const submitFunction = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    onSubmit?.(inputState);
  };

  return (
    <form onSubmit={submitFunction} style={{ color: "red" }}>
      <GlitchedInput
        inputState={inputState}
        setInputState={setInputState}
        label={label}
        placeholder={placeholder}
      />
      <GlitchedButton type="submit">Update</GlitchedButton>
    </form>
  );
};
