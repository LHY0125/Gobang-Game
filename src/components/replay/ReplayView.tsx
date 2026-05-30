import { useState, useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import BoardCanvas from '../board/BoardCanvas';
import StepSlider from './StepSlider';
import ReplayControls from './ReplayControls';

interface Props {
  onBackToMenu: () => void;
}

export default function ReplayView({ onBackToMenu }: Props) {
  const { t } = useTranslation();
  const moves = useGameStore((s) => s.moves);
  const [step, setStep] = useState(moves.length);
  const [isPlaying, setIsPlaying] = useState(false);

  useEffect(() => {
    if (!isPlaying) return;
    if (step >= moves.length) {
      setIsPlaying(false);
      return;
    }
    const timer = setInterval(() => setStep((s) => s + 1), 500);
    return () => clearInterval(timer);
  }, [isPlaying, step, moves.length]);

  return (
    <div className="replay-view">
      <div className="board-container">
        <BoardCanvas />
      </div>
      <StepSlider current={step} total={moves.length} onChange={setStep} />
      <div>{t('replay.step', { current: step, total: moves.length })}</div>
      <ReplayControls
        isPlaying={isPlaying}
        onTogglePlay={() => setIsPlaying(!isPlaying)}
        onPrev={() => setStep(Math.max(0, step - 1))}
        onNext={() => setStep(Math.min(moves.length, step + 1))}
      />
      <button onClick={onBackToMenu}>返回菜单</button>
    </div>
  );
}
