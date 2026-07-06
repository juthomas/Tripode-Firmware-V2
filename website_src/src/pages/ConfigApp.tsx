import { useEffect, useState, useCallback } from "react";
import { GlitchedBackground } from "../components/GlitchedBackground";
import { GlitchedTitle } from "../components/GlitchedTitle";
import "./config.css";

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

const fieldLabels: Record<keyof Settings, string> = {
  sta_ssid: "WiFi STA SSID",
  sta_pswd: "WiFi STA mot de passe",
  ap_ssid: "WiFi AP SSID",
  ap_pswd: "WiFi AP mot de passe",
  osc_target_ip: "OSC cible (IP ou .local)",
  osc_target_port: "OSC port",
  upd_target_ip: "UDP cible (IP ou .local)",
  upd_target_port: "UDP port sortant",
  upd_input_port: "UDP port entrant",
  tripode_id: "Tripode ID",
  fractal_state_pos_x: "Fractal pos X",
  fractal_state_pos_y: "Fractal pos Y",
  glyph_pos_x: "Glyph pos X",
  glyph_pos_y: "Glyph pos Y",
};

const fieldPlaceholders: Partial<Record<keyof Settings, string>> = {
  osc_target_ip: "192.168.1.10 ou macbook.local",
  upd_target_ip: "192.168.1.10 ou macbook.local",
};

const settingSections: { title: string; keys: (keyof Settings)[] }[] = [
  {
    title: "Réseau",
    keys: ["sta_ssid", "sta_pswd", "ap_ssid", "ap_pswd"],
  },
  {
    title: "Cibles OSC / UDP",
    keys: [
      "osc_target_ip",
      "osc_target_port",
      "upd_target_ip",
      "upd_target_port",
      "upd_input_port",
    ],
  },
  {
    title: "Orca",
    keys: [
      "fractal_state_pos_x",
      "fractal_state_pos_y",
      "glyph_pos_x",
      "glyph_pos_y",
    ],
  },
  {
    title: "Identité",
    keys: ["tripode_id"],
  },
];

function SettingRow({
  label,
  value,
  placeholder,
  type = "text",
  onSubmit,
}: {
  label: string;
  value: string | number;
  placeholder?: string;
  type?: "text" | "number";
  onSubmit: (v: string | number) => void;
}) {
  const [draft, setDraft] = useState(String(value));

  useEffect(() => {
    setDraft(String(value));
  }, [value]);

  const submit = () => {
    onSubmit(type === "number" ? Number(draft) : draft);
  };

  return (
    <>
      <label className="settings-label">{label}</label>
      <input
        className="settings-input"
        type={type}
        value={draft}
        placeholder={placeholder}
        onChange={(e) => setDraft(e.target.value)}
        onKeyDown={(e) => e.key === "Enter" && submit()}
      />
      <button type="button" className="settings-btn" onClick={submit}>
        Update
      </button>
    </>
  );
}

export const ConfigApp = (): JSX.Element => {
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
      <div className="config-page">
        <div className="config-header">
          <GlitchedTitle />
          <a className="config-doc-link" href="/config/doc">
            Documentation UDP/OSC
          </a>
        </div>
        <div className="config-layout">
          <div className="config-settings">
            {settingSections.map((section) => (
              <div key={section.title} className="config-section">
                <h2 className="config-section-title">{section.title}</h2>
                <div className="settings-grid">
                  {section.keys.map((key) => {
                    const value = data.settings[key];
                    if (value === undefined) return null;
                    const isNumber = typeof value === "number";
                    return (
                      <SettingRow
                        key={key}
                        label={fieldLabels[key]}
                        value={value}
                        placeholder={fieldPlaceholders[key]}
                        type={isNumber ? "number" : "text"}
                        onSubmit={(v) => updateSetting(key, v)}
                      />
                    );
                  })}
                </div>
              </div>
            ))}
          </div>

          <div className="config-section">
            <h2 className="config-section-title">Signaux</h2>
            <div className="signals-panel">
              {commands.map((cmd, index) => (
                <div key={index} className="signal-row">
                  <input
                    value={cmd.value}
                    placeholder="commande"
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
              <div className="signals-actions">
                <button type="button" onClick={addCommand}>
                  +
                </button>
                <button type="button" onClick={removeCommand}>
                  -
                </button>
                <button type="button" onClick={saveCommands}>
                  Save commands
                </button>
              </div>
            </div>
          </div>
        </div>
      </div>
    </>
  );
};
