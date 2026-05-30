interface Props {
  current: number;
  total: number;
  onChange: (step: number) => void;
}

export default function StepSlider({ current, total, onChange }: Props) {
  return (
    <input
      type="range"
      min={0}
      max={total}
      value={current}
      onChange={(e) => onChange(Number(e.target.value))}
      className="step-slider"
    />
  );
}
