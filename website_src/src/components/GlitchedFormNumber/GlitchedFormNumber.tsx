import { FormEvent, useState } from "react";
import { GlitchedButton } from "../GlitchedButton";
import { GlitchedInputNumber } from "../GlitchedInputNumber/GlitchedInputNumber";

interface GlitchedFormProps {
  label?: string;
  placeholder?: string;
  value?:number;
}

export const GlitchedFormNumber = ({ label = 'default', placeholder=label, value }: GlitchedFormProps): JSX.Element => {
  const [inputState, setInputState] = useState(value || 0);

  const submitFunction = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    console.log("[DEBUG] Submit event :", inputState);
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
