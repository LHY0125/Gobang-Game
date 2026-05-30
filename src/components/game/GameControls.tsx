import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';

interface Props {
  onBackToMenu: () => void;
}

export default function GameControls({ onBackToMenu }: Props) {
  const { t } = useTranslation();
  const undo = useGameStore((s) => s.undo);
  const mode = useGameStore((s) => s.mode);
  const status = useGameStore((s) => s.status);

  const handleUndo = () => {
    undo(1);
  };

  return (
    <div className="game-controls">
      <button onClick={handleUndo} disabled={status === 'game_over'}>
        {t('game.undo')}
      </button>
      <button onClick={onBackToMenu}>{t('game.new_game')}</button>
    </div>
  );
}
