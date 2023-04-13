import { useEffect, useState } from "react";
import { GlitchedBackground } from "./components/GlitchedBackground";
import { GlitchedForm } from "./components/GlitchedForm";
import { GlitchedTitle } from "./components/GlitchedTitle";


function App() {

  const [socket, setSocket] = useState<WebSocket | null>(null);
  useEffect(() => {
    const newSocket = new WebSocket('ws://' + window.location.hostname + '/ws');
    newSocket.addEventListener('open', (event) => {
      console.log('WebSocket connection established');
    });

    newSocket.addEventListener('message', (event) => {
      console.log(`Received message: ${event.data}`);
    });

    newSocket.addEventListener('error', (event) => {
      console.error(`WebSocket error: ${event}`);
    });

    setSocket(newSocket);

    // nettoyage lors de la destruction du composant
    return () => {
      if (socket) {
        socket.close();
      }
    };

  }, [])

  const sendMessage = () => {
    if (socket) {
      // socket.send(`{
      //   "settings": {
      //     "upd_target_ip": "192.168.4.2",
      //     "upd_target_port": 49160,
      //     "upd_input_port": 49141,
      //     "osc_target_ip": "192.168.4.2",
      //     "osc_target_port": 2002,
      //     "sta_ssid": "tripodesAP",
      //     "sta_pswd": "44448888",
      //     "ap_ssid": "tripodesAP",
      //     "ap_pswd": "44448888",
      //     "tripode_id": "1_1"
      //   },
      //   "signals": [
      //     {
      //       "value": "write:;#0D300I255P1;12;12",
      //       "type": "udp"
      //     }
      //   ]
      // }`);
      socket.send(`HIII`);
    }
  };


  return (
    <>
      <GlitchedBackground />
      <div style={{height: '100vh', display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
        <GlitchedTitle />
        <button onClick={sendMessage}>Envoyer un message</button>

        <GlitchedForm />
      </div>
    </>
  );
}

export default App;
