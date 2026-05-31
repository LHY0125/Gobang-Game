import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { MIN_BOARD_SIZE, MAX_BOARD_SIZE } from '../../core/constants';
import type { GameConfig } from '../../core/types';

interface Props {
  onBack: () => void;
  onStart: () => void;
}

export default function LocalGameSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [boardSize, setBoardSize] = useState(15);

  const handleStart = async () => {
    const config: GameConfig = {
      boardSize,
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
      <label>
        {t('settings.board_size')}:
        <select value={boardSize} onChange={(e) => setBoardSize(Number(e.target.value))}>
          {Array.from({ length: MAX_BOARD_SIZE - MIN_BOARD_SIZE + 1 }, (_, i) => MIN_BOARD_SIZE + i).map((s) => (
            <option key={s} value={s}>{s}&times;{s}</option>
          ))}
        </select>
      </label>
      <div className="setup-actions">
        <button onClick={handleStart}>{t('game.new_game')}</button>
        <button onClick={onBack}>{t('common.back')}</button>
      </div>
    </div>
  );
}
