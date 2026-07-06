import { useEffect, useState, useCallback } from "react";
import { GlitchedBackground } from "./components/GlitchedBackground";
import { GlitchedForm } from "./components/GlitchedForm";
import { GlitchedTitle } from "./components/GlitchedTitle";
import { GlitchedFormNumber } from "./components/GlitchedFormNumber/GlitchedFormNumber";
import { GlitchedCommandForm } from "./components/GlitchedCommandForm";

interface Signal {
  value: string;
  type: "udp" | "osc";
}

interface Settings {
  upd_target_ip: string;
  upd_target_port: number;
  upd_input_port: number;
  osc_target_ip: string;
  osc_target_port: number;
  sta_ssid: string;
  sta_pswd: string;
  ap_ssid: string;
  ap_pswd: string;
  tripode_id: string;
  fractal_state_pos_x?: number;
  fractal_state_pos_y?: number;
  glyph_pos_x?: number;
  glyph_pos_y?: number;
}

interface DataInterface {
  settings: Settings;
  signals: Signal[];
}

const defaultData: DataInterface = {
  settings: {
    upd_target_ip: "192.168.4.2",
    upd_target_port: 49160,
    upd_input_port: 49141,
    osc_target_ip: "192.168.4.2",
    osc_target_port: 2002,
    sta_ssid: "tripodesAP",
    sta_pswd: "44448888",
    ap_ssid: "tripodesAP",
    ap_pswd: "44448888",
    tripode_id: "1_1",
    fractal_state_pos_x: 0,
    fractal_state_pos_y: 10,
    glyph_pos_x: 48,
    glyph_pos_y: 6,
  },
  signals: [{ value: "write:;#0D300I255P1;12;12", type: "udp" }],
};

function App() {
  const [socket, setSocket] = useState<WebSocket | null>(null);
  const [data, setData] = useState<DataInterface>(defaultData);
  const [commands, setCommands] = useState<Signal[]>(defaultData.signals);

  useEffect(() => {
    const host = window.location.hostname || "4.3.2.1";
    const newSocket = new WebSocket(`ws://${host}/ws`);

    newSocket.addEventListener("open", () => {
      console.log("WebSocket connected");
    });

    newSocket.addEventListener("message", (event) => {
      try {
        const parsed = JSON.parse(event.data) as DataInterface;
        if (parsed.settings) setData(parsed);
        if (parsed.signals) setCommands(parsed.signals);
      } catch (e) {
        console.error("Invalid WS payload", e);
      }
    });

    newSocket.addEventListener("error", () => {
      window.location.reload();
    });

    setSocket(newSocket);
    return () => newSocket.close();
  }, []);

  const sendJson = useCallback(
    (payload: object) => {
      if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify(payload));
      }
    },
    [socket]
  );

  const updateSetting = (key: keyof Settings, value: string | number) => {
    const next = {
      ...data,
      settings: { ...data.settings, [key]: value },
    };
    setData(next);
    sendJson({ settings: { [key]: value } });
  };

  const saveCommands = () => {
    sendJson({ signals: commands });
  };

  const addCommand = () => {
    setCommands([...commands, { value: "", type: "udp" }]);
  };

  const removeCommand = () => {
    if (commands.length > 1) setCommands(commands.slice(0, -1));
  };

  return (
    <>
      <GlitchedBackground />
      <div
        style={{
          height: "100vh",
          display: "flex",
          flexDirection: "column",
          justifyContent: "center",
          alignItems: "center",
        }}
      >
        <GlitchedTitle />
        <div style={{ display: "flex", gap: "24px" }}>
          <div style={{ display: "flex", flexDirection: "column" }}>
            {Object.entries(data.settings).map(([key, value]) =>
              typeof value === "number" ? (
                <GlitchedFormNumber
                  key={key}
                  label={key}
                  value={value}
                  onSubmit={(v) =>
                    updateSetting(key as keyof Settings, Number(v))
                  }
                />
              ) : (
                <GlitchedForm
                  key={key}
                  label={key}
                  value={value}
                  onSubmit={(v) => updateSetting(key as keyof Settings, v)}
                />
              )
            )}
          </div>
          <div style={{ display: "flex", flexDirection: "column", gap: 8 }}>
            {commands.map((cmd, index) => (
              <div key={index} style={{ display: "flex", gap: 8 }}>
                <input
                  value={cmd.value}
                  onChange={(e) => {
                    const next = [...commands];
                    next[index] = { ...next[index], value: e.target.value };
                    setCommands(next);
                  }}
                />
                <select
                  value={cmd.type}
                  onChange={(e) => {
                    const next = [...commands];
                    next[index] = {
                      ...next[index],
                      type: e.target.value as "udp" | "osc",
                    };
                    setCommands(next);
                  }}
                >
                  <option value="udp">UDP</option>
                  <option value="osc">OSC</option>
                </select>
              </div>
            ))}
            <button onClick={addCommand}>+</button>
            <button onClick={removeCommand}>-</button>
            <button onClick={saveCommands}>Save commands</button>
          </div>
        </div>
      </div>
    </>
  );
}

export default App;
