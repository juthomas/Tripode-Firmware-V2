import { useEffect, useState, useCallback, useRef } from "react";
import { GlitchedBackground } from "../components/GlitchedBackground";
import { GlitchedTitle } from "../components/GlitchedTitle";
import "./config.css";

interface Signal {
  value: string;
  type: "udp" | "osc";
  enabled?: boolean;
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
  gyro_norm_min?: number;
  gyro_norm_max?: number;
  accel_norm_min?: number;
  accel_norm_max?: number;
  mag_norm_min?: number;
  mag_norm_max?: number;
  signal_poll_ms?: number;
}

interface DataInterface {
  settings: Settings;
  signals: Signal[];
  meta?: { saved?: boolean };
}

type SaveState = "idle" | "pending" | "saved" | "error";
type WsStatus = "connecting" | "open" | "closed" | "error";
type SignalsSaveState = "idle" | "pending" | "saved" | "error";

const SAVE_ACK_MS = 5000;
const SAVED_FLASH_MS = 2500;
const TOAST_MS = 3000;

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
    gyro_norm_min: 0,
    gyro_norm_max: 35,
    accel_norm_min: 0,
    accel_norm_max: 35,
    mag_norm_min: 0,
    mag_norm_max: 35,
    signal_poll_ms: 25,
  },
  signals: [{ value: "write:;#0D300I255P1;12;12", type: "udp", enabled: true }],
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
  gyro_norm_min: "Gyro norm min",
  gyro_norm_max: "Gyro norm max",
  accel_norm_min: "Accel norm min",
  accel_norm_max: "Accel norm max",
  mag_norm_min: "Magneto norm min",
  mag_norm_max: "Magneto norm max",
  signal_poll_ms: "Intervalle envoi signaux (ms)",
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
      "signal_poll_ms",
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
    title: "Normalisation capteurs",
    keys: [
      "gyro_norm_min",
      "gyro_norm_max",
      "accel_norm_min",
      "accel_norm_max",
      "mag_norm_min",
      "mag_norm_max",
    ],
  },
  {
    title: "Identité",
    keys: ["tripode_id"],
  },
];

function valuesMatch(
  a: string | number | undefined,
  b: string | number
): boolean {
  if (a === undefined) return false;
  if (typeof b === "number") return Number(a) === b;
  return String(a) === String(b);
}

function isSignalEnabled(sig: Signal): boolean {
  return sig.enabled !== false;
}

function signalsMatch(a: Signal[], b: Signal[]): boolean {
  if (a.length !== b.length) return false;
  return a.every(
    (sig, i) =>
      sig.value === b[i].value &&
      sig.type === b[i].type &&
      isSignalEnabled(sig) === isSignalEnabled(b[i])
  );
}

function SettingRow({
  label,
  value,
  placeholder,
  type = "text",
  saveState,
  onSubmit,
}: {
  label: string;
  value: string | number;
  placeholder?: string;
  type?: "text" | "number";
  saveState: SaveState;
  onSubmit: (v: string | number) => void;
}) {
  const [draft, setDraft] = useState(String(value));

  useEffect(() => {
    setDraft(String(value));
  }, [value]);

  const submit = () => {
    onSubmit(type === "number" ? Number(draft) : draft);
  };

  const btnClass =
    saveState === "saved"
      ? "settings-btn settings-btn-saved"
      : saveState === "error"
        ? "settings-btn settings-btn-error"
        : "settings-btn";

  const btnLabel =
    saveState === "pending"
      ? "…"
      : saveState === "saved"
        ? "✓"
        : saveState === "error"
          ? "!"
          : "Enregistrer";

  return (
    <>
      <label className="settings-label">{label}</label>
      <input
        className="settings-input"
        type={type}
        value={draft}
        placeholder={placeholder}
        disabled={saveState === "pending"}
        onChange={(e) => setDraft(e.target.value)}
        onKeyDown={(e) => e.key === "Enter" && submit()}
      />
      <button
        type="button"
        className={btnClass}
        onClick={submit}
        disabled={saveState === "pending"}
      >
        {btnLabel}
      </button>
    </>
  );
}

export const ConfigApp = (): JSX.Element => {
  const socketRef = useRef<WebSocket | null>(null);
  const reconnectDelayRef = useRef(3000);
  const reconnectTimerRef = useRef<number | null>(null);
  const pendingSettingsRef = useRef<
    Record<string, { value: string | number; timeoutId: number }>
  >({});
  const pendingSignalsRef = useRef<{
    signals: Signal[];
    timeoutId: number;
  } | null>(null);
  const savedTimersRef = useRef<Record<string, number>>({});

  const [wsStatus, setWsStatus] = useState<WsStatus>("connecting");
  const [data, setData] = useState<DataInterface>(defaultData);
  const [commands, setCommands] = useState<Signal[]>(defaultData.signals);
  const [fieldSaveState, setFieldSaveState] = useState<
    Record<string, SaveState>
  >({});
  const [signalsSaveState, setSignalsSaveState] =
    useState<SignalsSaveState>("idle");
  const [toast, setToast] = useState<{
    message: string;
    kind: "success" | "error";
  } | null>(null);

  const showToast = useCallback(
    (message: string, kind: "success" | "error") => {
      setToast({ message, kind });
      window.setTimeout(() => setToast(null), TOAST_MS);
    },
    []
  );

  const markFieldSaved = useCallback(
    (key: string, label: string) => {
      setFieldSaveState((prev) => ({ ...prev, [key]: "saved" }));
      showToast(`${label} enregistré`, "success");
      if (savedTimersRef.current[key])
        window.clearTimeout(savedTimersRef.current[key]);
      savedTimersRef.current[key] = window.setTimeout(() => {
        setFieldSaveState((prev) => ({ ...prev, [key]: "idle" }));
      }, SAVED_FLASH_MS);
    },
    [showToast]
  );

  const markFieldError = useCallback(
    (key: string) => {
      setFieldSaveState((prev) => ({ ...prev, [key]: "error" }));
    },
    []
  );

  const handleWsMessage = useCallback(
    (parsed: DataInterface) => {
      if (parsed.settings) setData(parsed);
      if (parsed.signals) setCommands(parsed.signals);

      const pendingSettings = pendingSettingsRef.current;
      for (const key of Object.keys(pendingSettings)) {
        const pending = pendingSettings[key];
        const deviceValue = parsed.settings?.[key as keyof Settings];
        if (
          parsed.meta?.saved === true &&
          valuesMatch(deviceValue, pending.value)
        ) {
          window.clearTimeout(pending.timeoutId);
          delete pendingSettingsRef.current[key];
          markFieldSaved(key, fieldLabels[key as keyof Settings] ?? key);
        }
      }

      const pendingSignals = pendingSignalsRef.current;
      if (
        pendingSignals &&
        parsed.signals &&
        parsed.meta?.saved === true &&
        signalsMatch(parsed.signals, pendingSignals.signals)
      ) {
        window.clearTimeout(pendingSignals.timeoutId);
        pendingSignalsRef.current = null;
        setSignalsSaveState("saved");
        showToast("Signaux enregistrés", "success");
        window.setTimeout(() => setSignalsSaveState("idle"), SAVED_FLASH_MS);
      }
    },
    [markFieldSaved, showToast]
  );

  const connectWs = useCallback(() => {
    const existing = socketRef.current;
    if (existing?.readyState === WebSocket.OPEN) return;
    if (existing?.readyState === WebSocket.CONNECTING) return;

    if (existing) {
      existing.onclose = null;
      existing.close();
      socketRef.current = null;
    }

    setWsStatus("connecting");
    const host = window.location.hostname || "4.3.2.1";
    const newSocket = new WebSocket(`ws://${host}/ws`);
    socketRef.current = newSocket;

    newSocket.addEventListener("open", () => {
      reconnectDelayRef.current = 3000;
      setWsStatus("open");
    });

    newSocket.addEventListener("message", (event) => {
      try {
        const parsed = JSON.parse(event.data as string) as DataInterface;
        handleWsMessage(parsed);
      } catch (e) {
        console.error("Invalid WS payload", e);
      }
    });

    newSocket.addEventListener("error", () => {
      setWsStatus("error");
    });

    newSocket.addEventListener("close", () => {
      setWsStatus("closed");
      if (socketRef.current === newSocket) socketRef.current = null;

      if (reconnectTimerRef.current) return;
      if (document.visibilityState === "hidden") return;

      const delay = reconnectDelayRef.current;
      reconnectTimerRef.current = window.setTimeout(() => {
        reconnectTimerRef.current = null;
        connectWs();
        reconnectDelayRef.current = Math.min(reconnectDelayRef.current * 2, 12000);
      }, delay);
    });
  }, [handleWsMessage]);

  useEffect(() => {
    connectWs();

    const onVisible = () => {
      if (
        document.visibilityState === "visible" &&
        socketRef.current?.readyState !== WebSocket.OPEN &&
        socketRef.current?.readyState !== WebSocket.CONNECTING &&
        !reconnectTimerRef.current
      ) {
        reconnectDelayRef.current = 3000;
        connectWs();
      }
    };
    document.addEventListener("visibilitychange", onVisible);

    return () => {
      document.removeEventListener("visibilitychange", onVisible);
      if (reconnectTimerRef.current) {
        window.clearTimeout(reconnectTimerRef.current);
        reconnectTimerRef.current = null;
      }
      socketRef.current?.close();
      socketRef.current = null;
      Object.values(pendingSettingsRef.current).forEach((p) =>
        window.clearTimeout(p.timeoutId)
      );
      if (pendingSignalsRef.current)
        window.clearTimeout(pendingSignalsRef.current.timeoutId);
      Object.values(savedTimersRef.current).forEach((id) =>
        window.clearTimeout(id)
      );
    };
  }, [connectWs]);

  const sendJson = useCallback((payload: object): boolean => {
    const ws = socketRef.current;
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify(payload));
      return true;
    }
    return false;
  }, []);

  const updateSetting = (key: keyof Settings, value: string | number) => {
    if (!sendJson({ settings: { [key]: value } })) {
      markFieldError(key);
      showToast("Non connecté — impossible d'enregistrer", "error");
      return;
    }

    setFieldSaveState((prev) => ({ ...prev, [key]: "pending" }));

    if (pendingSettingsRef.current[key])
      window.clearTimeout(pendingSettingsRef.current[key].timeoutId);

    const timeoutId = window.setTimeout(() => {
      delete pendingSettingsRef.current[key];
      markFieldError(key);
      showToast(`Échec : ${fieldLabels[key]}`, "error");
    }, SAVE_ACK_MS);

    pendingSettingsRef.current[key] = { value, timeoutId };
  };

  const saveCommands = () => {
    if (!sendJson({ signals: commands })) {
      setSignalsSaveState("error");
      showToast("Non connecté — signaux non enregistrés", "error");
      return;
    }

    setSignalsSaveState("pending");

    if (pendingSignalsRef.current)
      window.clearTimeout(pendingSignalsRef.current.timeoutId);

    const snapshot = commands.map((c) => ({ ...c }));
    const timeoutId = window.setTimeout(() => {
      pendingSignalsRef.current = null;
      setSignalsSaveState("error");
      showToast("Échec enregistrement des signaux", "error");
    }, SAVE_ACK_MS);

    pendingSignalsRef.current = { signals: snapshot, timeoutId };
  };

  const addCommand = () => {
    setCommands([...commands, { value: "", type: "udp", enabled: true }]);
  };

  const removeCommand = (index: number) => {
    if (commands.length <= 1) return;
    setCommands(commands.filter((_, i) => i !== index));
  };

  const wsBannerClass =
    wsStatus === "open"
      ? "config-ws-banner config-ws-banner-open"
      : wsStatus === "connecting"
        ? "config-ws-banner config-ws-banner-connecting"
        : "config-ws-banner config-ws-banner-error";

  const wsBannerText =
    wsStatus === "open"
      ? "Connecté au tripode"
      : wsStatus === "connecting"
        ? "Connexion au tripode…"
        : "Déconnecté — les modifications ne seront pas enregistrées";

  const signalsBtnClass =
    signalsSaveState === "saved"
      ? "signals-save-btn signals-save-btn-saved"
      : signalsSaveState === "error"
        ? "signals-save-btn signals-save-btn-error"
        : "signals-save-btn";

  const signalsBtnLabel =
    signalsSaveState === "pending"
      ? "Enregistrement…"
      : signalsSaveState === "saved"
        ? "Signaux enregistrés ✓"
        : signalsSaveState === "error"
          ? "Échec — réessayer"
          : "Enregistrer les signaux";

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

        <div className={`${wsBannerClass} config-ws-banner-wrap`}>
          <span>{wsBannerText}</span>
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
                        saveState={fieldSaveState[key] ?? "idle"}
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
            <p className="config-hint">
              Cliquez <strong>Enregistrer les signaux</strong> pour persister
              les modifications.
            </p>
            <div className="signals-panel">
              {commands.map((cmd, index) => (
                <div
                  key={index}
                  className={
                    isSignalEnabled(cmd)
                      ? "signal-row"
                      : "signal-row signal-row-disabled"
                  }
                >
                  <input
                    type="checkbox"
                    className="signal-enable-checkbox"
                    checked={isSignalEnabled(cmd)}
                    onChange={(e) => {
                      const next = [...commands];
                      next[index] = {
                        ...next[index],
                        enabled: e.target.checked,
                      };
                      setCommands(next);
                    }}
                    aria-label="Activer ce signal"
                    title="Activer / désactiver"
                  />
                  <input
                    value={cmd.value}
                    placeholder="write:1V{G:Y};0;0 ou TEST{G:Y:DEC}, {A}, {M:HEX:Z}"
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
                  <button
                    type="button"
                    className="signal-remove-btn"
                    onClick={() => removeCommand(index)}
                    disabled={commands.length <= 1}
                    aria-label="Supprimer ce signal"
                  >
                    -
                  </button>
                </div>
              ))}
              <div className="signals-actions">
                <button type="button" onClick={addCommand}>
                  +
                </button>
                <button
                  type="button"
                  className={signalsBtnClass}
                  onClick={saveCommands}
                  disabled={signalsSaveState === "pending"}
                >
                  {signalsBtnLabel}
                </button>
              </div>
            </div>
          </div>
        </div>
      </div>

      {toast && (
        <div
          className={`config-toast ${
            toast.kind === "success"
              ? "config-toast-success"
              : "config-toast-error"
          }`}
          role="status"
        >
          {toast.message}
        </div>
      )}
    </>
  );
};
