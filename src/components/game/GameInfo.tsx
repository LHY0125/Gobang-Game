import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';

export default function GameInfo() {
  const { t } = useTranslation();
  const currentColor = useGameStore((s) => s.currentColor);
  const status = useGameStore((s) => s.status);
  const winner = useGameStore((s) => s.winner);

  let text = '';
  if (status === 'game_over' && winner) {
    text = winner === 'Black' ? t('game.black_win') : t('game.white_win');
  } else if (status === 'ai_thinking') {
    text = t('game.ai_thinking');
  } else if (status === 'playing') {
    text = currentColor === 'Black' ? t('game.black_turn') : t('game.white_turn');
  }

  return <div className="game-info">{text}</div>;
}
