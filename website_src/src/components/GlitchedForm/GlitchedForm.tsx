import { FormEvent, useState } from "react";
import { GlitchedButton } from "../GlitchedButton";
import { GlitchedInput } from "../GlitchedInput";

export const GlitchedForm = (): JSX.Element => {
  const [inputState, setInputState] = useState("VALUE");

  const submitFunction = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    console.log("[DEBUG] Submit event :", inputState);
  };

  return (
    <form onSubmit={submitFunction} style={{ color: "red" }}>
      <GlitchedInput
        inputState={inputState}
        setInputState={setInputState}
        label="label"
        placeholder={"placeholder"}
      />
      <GlitchedButton type="submit">Update</GlitchedButton>
    </form>
  );
};
