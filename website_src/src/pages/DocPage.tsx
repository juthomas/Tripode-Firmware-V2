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
            À chaque cycle (~25 ms), le firmware lit les capteurs, exécute les
            signaux configurés, puis met à jour l&apos;écran.
          </p>
          <p>
            <code>capteurs → signals[] → UDP / OSC → affichage TFT</code>
          </p>
        </section>

        <section className="doc-section">
          <h2>Signaux configurables</h2>
          <p>
            La liste <code>signals[]</code> (section Signaux de cette interface)
            est la <strong>seule voie</strong> pour envoyer des valeurs capteur
            en réseau. Chaque entrée a :
          </p>
          <ul>
            <li>
              <code>value</code> — contenu du message (avec expressions capteur{" "}
              <code>{"{…}"}</code> pour injecter gyro / accel / magneto)
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
          <h2>Expressions capteur {"{…}"}</h2>
          <p>Syntaxe commune UDP et OSC :</p>
          <code className="doc-code-block">
            {"{<capteur>[:<axe>][:<encodage>]}"}
          </code>
          <ul>
            <li>
              <code>G</code> — gyroscope, <code>A</code> — accéléromètre,{" "}
              <code>M</code> — magnétomètre
            </li>
            <li>
              Axe (optionnel) : <code>X</code>, <code>Y</code>, <code>Z</code> —
              sans axe = moyenne des trois composantes
            </li>
            <li>
              Encodage UDP (optionnel) : <code>B35</code> (défaut),{" "}
              <code>DEC</code>, <code>HEX</code>
            </li>
            <li>
              Séparateur interne : <code>:</code> — l&apos;ordre des sous-paramètres
              est libre
            </li>
          </ul>
          <p>Exemples UDP :</p>
          <ul>
            <li>
              <code>TEST{"{G:Y:DEC}"}</code> — gyro Y, décimal normalisé
            </li>
            <li>
              <code>{"{A}"}</code> — moyenne accel, 1 caractère base35
            </li>
            <li>
              <code>write:1V{"{G:X}"}2V{"{G:Y}"}3V{"{G:Z}"};0;0</code> — gyro Orca
            </li>
          </ul>
          <p>Exemples OSC (float normalisé, encodage texte ignoré) :</p>
          <ul>
            <li>
              <code>/gyroscope_y:{"{G:Y}"}</code> → adresse{" "}
              <code>/gyroscope_y_{"{tripode_id}"}</code> + float gyro Y
            </li>
            <li>
              <code>/acc:{"{A}"}</code> → moyenne accel normalisée
            </li>
          </ul>
          <p>
            Un signal OSC sans bloc <code>{"{…}"}</code> n&apos;envoie rien.
            Plusieurs blocs <code>{"{…}"}</code> dans un même signal OSC =
            un message OSC par bloc.
          </p>
        </section>

        <section className="doc-section">
          <h2>Normalisation capteurs</h2>
          <p>
            La section <strong>Normalisation capteurs</strong> définit, pour
            chaque capteur, une plage de sortie <code>min</code> / <code>max</code>.
            La valeur brute est mappée depuis sa plage firmware fixe, puis
            clampée avant encodage UDP ou envoi OSC float.
          </p>
          <p>Plages brutes firmware (entrée) :</p>
          <ul>
            <li>Gyro : -37000 … 37000</li>
            <li>Accéléromètre : -40 … 40</li>
            <li>Magnétomètre : -100 … 100</li>
          </ul>
        </section>

        <section className="doc-section">
          <h2>UDP — protocole Orca</h2>
          <p>
            Les messages UDP sont du texte brut envoyés vers{" "}
            <code>upd_target_ip:upd_target_port</code>.
          </p>
          <p>Format générique pour écrire une ligne Orca :</p>
          <code className="doc-code-block">write:&lt;ligne&gt;;&lt;x&gt;;&lt;y&gt;</code>
          <p>Signaux par défaut inclus : glyph moteur, select, gyro Orca.</p>
        </section>

        <section className="doc-section">
          <h2>OSC</h2>
          <p>
            Chaque message OSC est un float normalisé envoyé vers{" "}
            <code>osc_target_ip:osc_target_port</code>.
          </p>
          <p>
            Adresse : <code>{"{prefix}_{tripode_id}"}</code> où{" "}
            <code>prefix</code> est la partie de <code>value</code> hors bloc{" "}
            <code>{"{…}"}</code>.
          </p>
          <p>
            Signaux OSC par défaut : <code>/gyroscope_x/y/z</code> avec{" "}
            <code>{"{G:X/Y/Z}"}</code>.
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
