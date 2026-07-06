import { FormEvent, useEffect, useState } from "react";
import { GlitchedButton } from "../GlitchedButton";
import { GlitchedInputNumber } from "../GlitchedInputNumber/GlitchedInputNumber";

interface GlitchedFormProps {
  label?: string;
  placeholder?: string;
  value?: number;
  onSubmit?: (value: number) => void;
}

export const GlitchedFormNumber = ({
  label = "default",
  placeholder = label,
  value,
  onSubmit,
}: GlitchedFormProps): JSX.Element => {
  const [inputState, setInputState] = useState(value || 0);

  useEffect(() => {
    if (value !== undefined) setInputState(value);
  }, [value]);

  const submitFunction = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    onSubmit?.(inputState);
  };

  return (
    <form onSubmit={submitFunction} style={{ color: "red" }}>
      <GlitchedInputNumber
        inputState={inputState}
        setInputState={setInputState}
        label={label}
        placeholder={placeholder}
      />
      <GlitchedButton type="submit">Update</GlitchedButton>
    </form>
  );
};
