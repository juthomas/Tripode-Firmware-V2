import "./index.css";

export interface GlitchedTitleProps {}

export const GlitchedTitle = ({}: GlitchedTitleProps): JSX.Element => {
  return (
    <div style={{ position: "relative" }}>
      <div className="title1">
        <h1 className="glitch1 title before">AS-SIMT</h1>
        <h1 className="glitch1 title">AS-SIMT</h1>
        <h1 className="glitch1 title after">AS-SIMT</h1>
      </div>
      <div className="title2">
        <h1 className="glitch2 title before">السمت</h1>
        <h1 className="glitch2 title">السمت</h1>
        <h1 className="glitch2 title after">السمت</h1>
      </div>
    </div>
  );
};
