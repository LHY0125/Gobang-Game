import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import type { GameConfig } from '../../core/types';

interface Props {
  onBack: () => void;
  onStart: () => void;
}

export default function LocalGameSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);

  const handleStart = async () => {
    const config: GameConfig = {
      boardSize: 15,
      useForbiddenRules: true,
      useTimer: false,
      timeLimitSecs: 60,
      aiDifficulty: 3,
      playerColor: 'Black',
      isServer: false,
      remoteAddress: '',
    };
    await startGame('Local', config);
    onStart();
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.local_game')}</h2>
      <div className="setup-actions">
        <button onClick={handleStart}>{t('game.new_game')}</button>
        <button onClick={onBack}>{t('common.back')}</button>
      </div>
    </div>
  );
}
