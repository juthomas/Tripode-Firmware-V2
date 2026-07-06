import React from "react";
import ReactDOM from "react-dom/client";
import { PortalPage } from "./pages/PortalPage";
import { ConfigApp } from "./pages/ConfigApp";
import { DocPage } from "./pages/DocPage";
import "./index.css";

const path = window.location.pathname;

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    {path.startsWith("/config/doc") ? (
      <DocPage />
    ) : path.startsWith("/config") ? (
      <ConfigApp />
    ) : (
      <PortalPage />
    )}
  </React.StrictMode>
);
