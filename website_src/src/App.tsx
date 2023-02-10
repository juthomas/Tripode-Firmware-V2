import { GlitchedBackground } from "./components/GlitchedBackground";
import { GlitchedForm } from "./components/GlitchedForm";
import { GlitchedTitle } from "./components/GlitchedTitle";

function App() {

  return (
    <>
      <GlitchedBackground />
      <div style={{height: '100vh', display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
        <GlitchedTitle />
        <GlitchedForm />
      </div>
    </>
  );
}

export default App;
