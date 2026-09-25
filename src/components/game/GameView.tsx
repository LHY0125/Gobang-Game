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
  const status = useGameStore((s) => s.status);
  const llmThinking = useGameStore((s) => s.llmThinking);
  const config = useGameStore((s) => s.config);
  const moves = useGameStore((s) => s.moves);
  const aiMove = useGameStore((s) => s.aiMove);
  const appendLlmThinking = useGameStore((s) => s.appendLlmThinking);
  const [connStatus, setConnStatus] = useState<string>('');

  // 后手 AI 先行：游戏启动时若轮到 AI，自动走第一步
  useEffect(() => {
    if (status === 'ai_thinking' && moves.length === 0) {
      aiMove();
    }
  }, [status, moves.length, aiMove]);

  // 监听网络事件
  useEffect(() => {
    if (mode !== 'Online') return;
    let unlisten1: (() => void) | undefined;
    let unlisten2: (() => void) | undefined;

    const setup = async () => {
      unlisten1 = await listen<string>('connection-status', (e) => setConnStatus(e.payload));
      unlisten2 = await listen<number>('listening-port', (e) =>
        setConnStatus('waiting:' + e.payload),
      );
    };
    setup();

    return () => {
      unlisten1?.();
      unlisten2?.();
    };
  }, [mode]);

  // 监听 LLM 流式事件
  useEffect(() => {
    let unlisten1: (() => void) | undefined;
    let unlisten2: (() => void) | undefined;

    const setup = async () => {
      unlisten1 = await listen<string>('llm-stream', (e) => {
        appendLlmThinking(e.payload);
      });
      unlisten2 = await listen<{ x: number; y: number }>('llm-move', (_e) => {
        // 坐标由 ai_move_llm 的返回值处理，此事件仅作通知
      });
    };
    setup();

    return () => {
      unlisten1?.();
      unlisten2?.();
    };
  }, [appendLlmThinking]);

  const isLlmThinking = status === 'ai_thinking' && config.useLlm;

  return (
    <div className="game-view">
      {mode === 'Online' && connStatus && (
        <div style={{ fontSize: 14, opacity: 0.8 }}>
          {connStatus.startsWith('waiting')
            ? t('game.waiting_opponent')
            : connStatus === 'connected'
              ? t('game.opponent_connected')
              : connStatus === 'disconnected'
                ? t('game.opponent_disconnected')
                : ''}
        </div>
      )}
      <GameInfo />
      <div className="board-container">
        <BoardCanvas />
      </div>
      {isLlmThinking && (
        <div className="llm-thinking">
          <div className="llm-thinking-header">{t('game.ai_thinking')}</div>
          <pre className="llm-thinking-text">{llmThinking}</pre>
        </div>
      )}
      <TimerDisplay />
      <GameControls onBackToMenu={onBackToMenu} />
    </div>
  );
}
