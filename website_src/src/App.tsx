import { useState } from "react";
import { GlitchedBackground } from "./components/GlitchedBackground";
import { GlitchedTitle } from "./components/GlitchedTitle";

function App() {
  const [count, setCount] = useState(0);

  return (
    <>
      <GlitchedBackground />
      <div style={{height: '100vh', display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
        <GlitchedTitle />
      </div>
    </>
  );
}

export default App;
