import { useEffect, useState } from "react";
import { GlitchedBackground } from "./components/GlitchedBackground";
import { GlitchedForm } from "./components/GlitchedForm";
import { GlitchedTitle } from "./components/GlitchedTitle";
import { GlitchedFormNumber } from "./components/GlitchedFormNumber/GlitchedFormNumber";
import { GlitchedCommandForm } from "./components/GlitchedCommandForm";

interface dataInterface {
  settings: {
    upd_target_ip: string
    upd_target_port: number
    upd_input_port: number
    osc_target_ip: string
    osc_target_port: number
    sta_ssid: string
    sta_pswd: string
    ap_ssid: string
    ap_pswd: string
    tripode_id: string
  },
  signals: [
    {
      value: string
      type: 'udp' | 'osc'
    }
  ]
}

// function isDataProps(data: any): data is dataInterface["settings"] {
//   if (!data)
//   return false;
//   const keys = Object.keys(data?.settings);
//   const props = Object.keys({} as dataInterface["settings"]);
//   return keys.length === props.length && props.every(prop => keys.includes(prop));
// }

function App() {
  const [socket, setSocket] = useState<WebSocket | null>(null);

  // console.log("[IS DATA ??]", isDataProps(`{
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
  //     "tripode_id": "1"
  //   },
  //   "signals": [
  //     {
  //       "value": "write:;#0D300I255P1;12;12",
  //       "type": "udp"
  //     }
  //   ]
  // }`))

  const [data, setData] = useState<dataInterface>(JSON.parse(
    `{
      "settings": {
        "upd_target_ip": "192.168.4.2",
        "upd_target_port": 49160,
        "upd_input_port": 49141,
        "osc_target_ip": "192.168.4.2",
        "osc_target_port": 2002,
        "sta_ssid": "tripodesAP",
        "sta_pswd": "44448888",
        "ap_ssid": "tripodesAP",
        "ap_pswd": "44448888",
        "tripode_id": "1"
      },
      "signals": [
        {
          "value": "write:;#0D300I255P1;12;12",
          "type": "udp"
        }
      ]
    }`)
  );
  useEffect(() => {
    // const newSocket = new WebSocket('ws://' + window.location.hostname + '/ws');
    const newSocket = new WebSocket('ws://' + "4.3.2.1" + '/ws');
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
      socket.send(`{
        "settings": {
          "upd_target_ip": "192.168.4.2",
          "upd_target_port": 49160,
          "upd_input_port": 49141,
          "osc_target_ip": "192.168.4.2",
          "osc_target_port": 2002,
          "sta_ssid": "tripodesAP",
          "sta_pswd": "44448888",
          "ap_ssid": "tripodesAP",
          "ap_pswd": "44448888",
          "tripode_id": "1_2"
        },
        "signals": [
          {
            "value": "write:;#0D300I255P1;12;12",
            "type": "udp"
          }
        ]
      }`);
      // socket.send(`HIII`);
    }
  };


  return (
    <>
      <GlitchedBackground />
      <div style={{ height: '100vh', display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
        <GlitchedTitle />
        <button onClick={sendMessage}>Envoyer un message</button>
        <div style={{ display: 'flex' }}>

          <div style={{ display: 'flex', flexDirection: 'column', justifyContent: 'center' }}>
            {Object.entries(data.settings).map((elem, index) => {
              if (typeof elem[1] === 'string')
                return <GlitchedForm key={index} label={elem[0]} value={elem[1]} />
              else if (typeof elem[1] === 'number')
                return <GlitchedFormNumber key={index} label={elem[0]} value={elem[1]} />
              return <></>
            })}
            <GlitchedForm />
          </div>
          <div style={{display: 'flex', flexDirection: 'column', justifyContent:'center'}}>
              <GlitchedCommandForm/>

          </div>
        </div>
      </div>
    </>
  );
}

export default App;
