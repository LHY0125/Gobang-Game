import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import type { Color, GameConfig } from '../../core/types';

interface Props {
  onBack: () => void;
  onStart: () => void;
}

export default function AiGameSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [difficulty, setDifficulty] = useState(3);
  const [playerColor, setPlayerColor] = useState<Color>('Black');
  const [useForbidden, setUseForbidden] = useState(true);

  const handleStart = async () => {
    const config: GameConfig = {
      boardSize: 15,
      useForbiddenRules: useForbidden,
      useTimer: false,
      timeLimitSecs: 60,
      aiDifficulty: difficulty,
      playerColor,
      isServer: false,
      remoteAddress: '',
    };
    await startGame('VsAi', config);
    onStart();
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.ai_game')}</h2>
      <label>
        {t('settings.difficulty')}:
        <select value={difficulty} onChange={(e) => setDifficulty(Number(e.target.value))}>
          {[1, 2, 3, 4, 5].map((d) => (
            <option key={d} value={d}>{d}</option>
          ))}
        </select>
      </label>
      <label>
        先手:
        <select value={playerColor} onChange={(e) => setPlayerColor(e.target.value as Color)}>
          <option value="Black">黑棋 (先手)</option>
          <option value="White">白棋 (后手)</option>
        </select>
      </label>
      <label>
        <input type="checkbox" checked={useForbidden} onChange={(e) => setUseForbidden(e.target.checked)} />
        {t('settings.forbidden_rules')}
      </label>
      <div className="setup-actions">
        <button onClick={handleStart}>{t('game.new_game')}</button>
        <button onClick={onBack}>返回</button>
      </div>
    </div>
  );
}
