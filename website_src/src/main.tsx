import React from "react";
import ReactDOM from "react-dom/client";
import { PortalPage } from "./pages/PortalPage";
import { ConfigApp } from "./pages/ConfigApp";
import { DocPage } from "./pages/DocPage";
import "./index.css";

const path = window.location.pathname;
const isCaptivePortalHost = window.location.hostname === "4.3.2.1";

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    {path.startsWith("/config/doc") ? (
      <DocPage />
    ) : path.startsWith("/config") ? (
      <ConfigApp />
    ) : isCaptivePortalHost ? (
      <PortalPage />
    ) : (
      <ConfigApp />
    )}
  </React.StrictMode>
);
