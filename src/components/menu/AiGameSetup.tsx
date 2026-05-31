import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { MIN_BOARD_SIZE, MAX_BOARD_SIZE } from '../../core/constants';
import type { Color, GameConfig } from '../../core/types';

interface Props {
  onBack: () => void;
  onStart: () => void;
}

export default function AiGameSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [boardSize, setBoardSize] = useState(15);
  const [difficulty, setDifficulty] = useState(3);
  const [playerColor, setPlayerColor] = useState<Color>('Black');
  const [useForbidden, setUseForbidden] = useState(true);

  const handleStart = async () => {
    const config: GameConfig = {
      boardSize,
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
        {t('settings.board_size')}:
        <select value={boardSize} onChange={(e) => setBoardSize(Number(e.target.value))}>
          {Array.from({ length: MAX_BOARD_SIZE - MIN_BOARD_SIZE + 1 }, (_, i) => MIN_BOARD_SIZE + i).map((s) => (
            <option key={s} value={s}>{s}&times;{s}</option>
          ))}
        </select>
      </label>
      <label>
        {t('settings.difficulty')}:
        <select value={difficulty} onChange={(e) => setDifficulty(Number(e.target.value))}>
          {[1, 2, 3, 4, 5].map((d) => (
            <option key={d} value={d}>{d}</option>
          ))}
        </select>
      </label>
      <label>
        {t('ai_setup.first_player')}:
        <select value={playerColor} onChange={(e) => setPlayerColor(e.target.value as Color)}>
          <option value="Black">{t('ai_setup.black_first')}</option>
          <option value="White">{t('ai_setup.white_second')}</option>
        </select>
      </label>
      <label>
        <input type="checkbox" checked={useForbidden} onChange={(e) => setUseForbidden(e.target.checked)} />
        {t('settings.forbidden_rules')}
      </label>
      <div className="setup-actions">
        <button onClick={handleStart}>{t('game.new_game')}</button>
        <button onClick={onBack}>{t('common.back')}</button>
      </div>
    </div>
  );
}
