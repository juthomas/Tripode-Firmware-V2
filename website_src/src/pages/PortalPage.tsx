import { useState } from "react";
import "./portal.css";

const CAPTIVE_HOST = "4.3.2.1";

export const PortalPage = (): JSX.Element => {
  const hostname = window.location.hostname || CAPTIVE_HOST;
  const configUrl = `http://${hostname}/config`;
  const [copied, setCopied] = useState(false);

  const copyConfigUrl = async () => {
    try {
      await navigator.clipboard.writeText(configUrl);
    } catch {
      const input = document.createElement("input");
      input.value = configUrl;
      document.body.appendChild(input);
      input.select();
      document.execCommand("copy");
      document.body.removeChild(input);
    }
    setCopied(true);
    setTimeout(() => setCopied(false), 2500);
  };

  const finishPortal = () => {
    window.location.href = "/portal/done";
  };

  return (
    <div className="portal-page">
      <div className="portal-card">
        <h1 className="portal-title">TRIPODE</h1>
        <p className="portal-tagline">
          Configurez le tripode dans le navigateur.
        </p>

        <code className="portal-url">{configUrl}</code>

        <div className="portal-actions">
          <a className="portal-btn portal-btn-primary" href="/config">
            Ouvrir la configuration
          </a>
          <button
            type="button"
            className="portal-btn portal-btn-done"
            onClick={finishPortal}
          >
            Terminé
          </button>
          <button
            type="button"
            className="portal-btn"
            onClick={copyConfigUrl}
          >
            {copied ? "Copié" : "Copier l'adresse"}
          </button>
        </div>

        <p className="portal-hint">
          Appuyez sur <strong>Terminé</strong> pour fermer le portail captif et
          retrouver votre connexion internet.
        </p>
      </div>
    </div>
  );
};
