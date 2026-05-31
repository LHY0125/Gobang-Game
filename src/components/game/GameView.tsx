import { useState, useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { listen } from '@tauri-apps/api/event';
import { useGameStore } from '../../store/gameStore';
import BoardCanvas from '../board/BoardCanvas';
import GameInfo from './GameInfo';
import GameControls from './GameControls';
import TimerDisplay from './TimerDisplay';

interface Props {
  onBackToMenu: () => void;
}

export default function GameView({ onBackToMenu }: Props) {
  const { t } = useTranslation();
  const mode = useGameStore((s) => s.mode);
  const [connStatus, setConnStatus] = useState<string>('');

  useEffect(() => {
    if (mode !== 'Online') return;
    let unlisten1: (() => void) | undefined;
    let unlisten2: (() => void) | undefined;

    const setup = async () => {
      unlisten1 = await listen<string>('connection-status', (e) => setConnStatus(e.payload));
      unlisten2 = await listen<number>('listening-port', (e) => setConnStatus('waiting:' + e.payload));
    };
    setup();

    return () => { unlisten1?.(); unlisten2?.(); };
  }, [mode]);

  return (
    <div className="game-view">
      {mode === 'Online' && connStatus && (
        <div style={{ fontSize: 14, opacity: 0.8 }}>
          {connStatus.startsWith('waiting') ? '等待对手加入...' :
           connStatus === 'connected' ? t('game.opponent_connected') :
           connStatus === 'disconnected' ? t('game.opponent_disconnected') : ''}
        </div>
      )}
      <GameInfo />
      <div className="board-container">
        <BoardCanvas />
      </div>
      <TimerDisplay />
      <GameControls onBackToMenu={onBackToMenu} />
    </div>
  );
}
