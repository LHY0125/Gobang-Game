import { useEffect, useRef, useCallback, useMemo } from 'react';
import { listen } from '@tauri-apps/api/event';
import { useGameStore, buildReplayBoard } from '../../store/gameStore';
import {
  computeBoardDimensions,
  canvasToBoard,
  renderBoard,
} from './board-renderer';

export default function BoardCanvas() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const board = useGameStore((s) => s.board);
  const boardSize = useGameStore((s) => s.boardSize);
  const status = useGameStore((s) => s.status);
  const mode = useGameStore((s) => s.mode);
  const placePiece = useGameStore((s) => s.placePiece);
  const aiMove = useGameStore((s) => s.aiMove);
  const moves = useGameStore((s) => s.moves);
  const replayStep = useGameStore((s) => s.replayStep);

  // 复盘模式下根据 replayStep 重建棋盘
  const displayBoard = useMemo(() => {
    if (mode === 'Replay') {
      return buildReplayBoard(boardSize, moves, replayStep);
    }
    return board;
  }, [mode, board, boardSize, moves, replayStep]);

  // 复盘模式下的最后一手
  const displayLastMove = useMemo(() => {
    if (mode !== 'Replay') {
      return moves.length > 0 ? moves[moves.length - 1].position : null;
    }
    if (replayStep > 0 && replayStep <= moves.length) {
      return moves[replayStep - 1].position;
    }
    return null;
  }, [mode, moves, replayStep]);

  const render = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.getBoundingClientRect();
    canvas.width = rect.width * dpr;
    canvas.height = rect.height * dpr;
    ctx.scale(dpr, dpr);

    const cfg = computeBoardDimensions(boardSize, rect.width, rect.height);
    renderBoard(ctx, displayBoard, cfg, displayLastMove);
  }, [displayBoard, boardSize, displayLastMove]);

  useEffect(() => {
    render();
    const handleResize = () => render();
    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, [render]);

  useEffect(() => {
    if (mode !== 'Online') return;
    let unlisten: (() => void) | undefined;

    const setup = async () => {
      unlisten = await listen<{ x: number; y: number }>('remote-move', (event) => {
        placePiece(event.payload.x, event.payload.y);
      });
    };
    setup();

    return () => { unlisten?.(); };
  }, [mode, placePiece]);

  const handleClick = useCallback(
    (e: React.MouseEvent<HTMLCanvasElement>) => {
      if (status !== 'playing') return;
      if (mode === 'VsAi' && moves.length % 2 === 1) return;
      if (mode === 'Replay') return;

      const canvas = canvasRef.current;
      if (!canvas) return;
      const rect = canvas.getBoundingClientRect();
      const cfg = computeBoardDimensions(boardSize, rect.width, rect.height);
      const pos = canvasToBoard(e.clientX - rect.left, e.clientY - rect.top, cfg);
      if (!pos) return;

      placePiece(pos.x, pos.y).then((result) => {
        if (!result.is_win && mode === 'VsAi') {
          setTimeout(() => aiMove(), 100);
        }
      });
    },
    [status, mode, boardSize, moves.length, placePiece, aiMove]
  );

  return (
    <canvas
      ref={canvasRef}
      onClick={handleClick}
      style={{
        width: '100%',
        height: '100%',
        cursor: status === 'playing' && mode !== 'Replay' ? 'pointer' : 'default',
      }}
    />
  );
}
