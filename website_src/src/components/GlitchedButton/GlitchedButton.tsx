export type GlitchedButtonProps = React.DetailedHTMLProps<
  React.ButtonHTMLAttributes<HTMLButtonElement>,
  HTMLButtonElement
>;

export const GlitchedButton = (props: GlitchedButtonProps): JSX.Element => {
  return <button {...props}></button>;
};
