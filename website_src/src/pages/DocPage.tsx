import "./doc.css";

export const DocPage = (): JSX.Element => {
  return (
    <div className="doc-page">
      <div className="doc-container">
        <header className="doc-header">
          <h1 className="doc-title">Documentation UDP / OSC</h1>
          <a className="doc-back" href="/config">
            Retour à la configuration
          </a>
        </header>

        <section className="doc-section">
          <h2>Boucle principale</h2>
          <p>
            À chaque cycle (~25 ms), le firmware lit les capteurs, calcule la
            valeur DFA, envoie éventuellement l&apos;OSC des capteurs, exécute
            les signaux configurés, puis met à jour l&apos;écran.
          </p>
          <p>
            <code>capteurs → DFA → OSC (si actif) → signals[] → affichage TFT</code>
          </p>
        </section>

        <section className="doc-section">
          <h2>Signaux configurables</h2>
          <p>
            La liste <code>signals[]</code> (section Signaux de cette interface)
            définit des messages envoyés en boucle. Chaque entrée a :
          </p>
          <ul>
            <li>
              <code>value</code> — contenu du message (avec placeholders
              optionnels)
            </li>
            <li>
              <code>type</code> — <code>udp</code> ou <code>osc</code>
            </li>
          </ul>
          <p>
            L&apos;envoi dépend des flags hardware : bouton gauche = toggle UDP,
            double-clic gauche = toggle OSC. Un signal n&apos;est émis que si son
            type est actif.
          </p>
        </section>

        <section className="doc-section">
          <h2>Placeholders</h2>
          <p>
            Dans <code>value</code>, ces tokens sont remplacés à chaque cycle
            avant envoi :
          </p>
          <ul>
            <li>
              <code>{"{gx}"}</code>, <code>{"{gy}"}</code>, <code>{"{gz}"}</code>{" "}
              — gyroscope (entiers)
            </li>
            <li>
              <code>{"{ax}"}</code>, <code>{"{ay}"}</code>, <code>{"{az}"}</code>{" "}
              — accéléromètre (float)
            </li>
            <li>
              <code>{"{dfa}"}</code> — valeur DFA courante
            </li>
          </ul>
        </section>

        <section className="doc-section">
          <h2>UDP — protocole Orca</h2>
          <p>
            Les messages UDP sont du texte brut envoyés vers{" "}
            <code>upd_target_ip:upd_target_port</code> (Orca ou autre cible).
          </p>
          <p>Format générique pour écrire une ligne Orca :</p>
          <code className="doc-code-block">write:&lt;ligne&gt;;&lt;x&gt;;&lt;y&gt;</code>
          <p>Exemple par défaut (moteur / glyph) :</p>
          <code className="doc-code-block">write:;#0D300I255P1;12;12</code>
          <p>Messages UDP automatiques quand l&apos;envoi UDP est actif :</p>
          <ul>
            <li>
              <code>select:0;0</code> puis{" "}
              <code>write:1V…2V…3V…;0;0</code> — axes gyro mappés en base35
              (0–9, a–z)
            </li>
            <li>
              Fractal state — alpha DFA encodé en base35, position définie par{" "}
              <code>fractal_state_pos_x/y</code>
            </li>
          </ul>
        </section>

        <section className="doc-section">
          <h2>OSC</h2>
          <p>
            Chaque message OSC est un float envoyé vers{" "}
            <code>osc_target_ip:osc_target_port</code>.
          </p>
          <p>
            Adresse : <code>{"{prefix}_{tripode_id}"}</code> (ex.{" "}
            <code>/dfa_1_1</code>).
          </p>
          <p>Flux automatique si OSC actif :</p>
          <ul>
            <li>
              <code>/dfa</code>, <code>/accelerometer_x/y/z</code>, norme accél
            </li>
            <li>
              <code>/gyroscope_x/y/z</code>, norme gyro
            </li>
            <li>
              <code>/magnetometer_x/y/z</code>
            </li>
          </ul>
          <p>
            Pour un signal <code>type: osc</code>, <code>value</code> est le
            préfixe OSC (ex. <code>/custom</code> → <code>/custom_1_1</code>).
          </p>
        </section>

        <section className="doc-section">
          <h2>DFA</h2>
          <p>
            Detrended Fluctuation Analysis — mesure la variabilité / complexité
            du signal capteur sur plusieurs échelles. Utilisée pour le fractal
            state Orca et le placeholder <code>{"{dfa}"}</code>.
          </p>
        </section>

        <section className="doc-section">
          <h2>Entrée UDP</h2>
          <p>
            Le port <code>upd_input_port</code> écoute les commandes entrantes
            (ex. contrôle moteurs). Les paquets reçus sont traités par le
            firmware et peuvent piloter les sorties PWM.
          </p>
        </section>
      </div>
    </div>
  );
};
