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
  const replayStep = useGameStore((s) => s.replayStep);
  const setReplayStep = useGameStore((s) => s.setReplayStep);
  const [isPlaying, setIsPlaying] = useState(false);

  const step = replayStep;

  useEffect(() => {
    if (!isPlaying) return;
    if (step >= moves.length) {
      setIsPlaying(false);
      return;
    }
    const timer = setInterval(() => setReplayStep(step + 1), 500);
    return () => clearInterval(timer);
  }, [isPlaying, step, moves.length, setReplayStep]);

  return (
    <div className="replay-view">
      <div className="board-container">
        <BoardCanvas />
      </div>
      <StepSlider current={step} total={moves.length} onChange={setReplayStep} />
      <div>{t('replay.step', { current: step, total: moves.length })}</div>
      <ReplayControls
        isPlaying={isPlaying}
        onTogglePlay={() => setIsPlaying(!isPlaying)}
        onPrev={() => setReplayStep(Math.max(0, step - 1))}
        onNext={() => setReplayStep(Math.min(moves.length, step + 1))}
      />
      <button onClick={onBackToMenu}>{t('common.back_to_menu')}</button>
    </div>
  );
}
