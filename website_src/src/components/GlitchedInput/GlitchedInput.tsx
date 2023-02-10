export interface GlitchedInputProps {
  label: string;
  placeholder?: string;
  inputState: string;
  setInputState: React.Dispatch<React.SetStateAction<string>>;
}

export const GlitchedInput = ({
  label,
  placeholder,
  inputState,
  setInputState,
}: GlitchedInputProps): JSX.Element => {
  return (
    <div>
      <label>{label}</label>
      <input
        type="text"
        onChange={(e) => setInputState(e.target.value)}
        placeholder={placeholder}
        value={inputState}
        required
      ></input>
    </div>
  );
};
