import { useEffect, useRef, useCallback } from 'react';
import { useGameStore } from '../../store/gameStore';
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

  const lastMove = moves.length > 0 ? moves[moves.length - 1].position : null;

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
    renderBoard(ctx, board, cfg, lastMove);
  }, [board, boardSize, lastMove]);

  useEffect(() => {
    render();
    const handleResize = () => render();
    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, [render]);

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
