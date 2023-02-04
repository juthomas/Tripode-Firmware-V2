import { useState } from "react";
import { GlitchedBackground } from "./components/GlitchedBackground";

function App() {
  const [count, setCount] = useState(0);

  return (
      <GlitchedBackground/>
  );
}

export default App;
