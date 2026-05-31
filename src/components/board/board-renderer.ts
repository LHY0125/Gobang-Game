import type { CellState, Position } from '../../core/types';

export interface RenderConfig {
  cellSize: number;
  padding: number;
  boardSize: number;
}

/**
 * 根据棋盘大小动态生成星位坐标。
 * 标准五子棋/围棋星位: 四角和中心及边中点。
 * 9x9 起可用，更小的棋盘仅使用中心点。
 */
export function computeStarPoints(boardSize: number): [number, number][] {
  if (boardSize < 9) {
    const mid = Math.floor(boardSize / 2);
    return [[mid, mid]];
  }
  const a = Math.min(3, Math.floor(boardSize / 4));
  const b = Math.floor(boardSize / 2);
  const c = boardSize - 1 - a;
  return [
    [a, a], [a, b], [a, c],
    [b, a], [b, b], [b, c],
    [c, a], [c, b], [c, c],
  ];
}

export function computeBoardDimensions(boardSize: number, canvasWidth: number, canvasHeight: number): RenderConfig {
  const maxBoardPixelSize = Math.min(canvasWidth, canvasHeight) * 0.85;
  const cellSize = Math.floor(maxBoardPixelSize / (boardSize - 1));
  const actualBoardPixelSize = cellSize * (boardSize - 1);
  const padding = Math.floor((Math.min(canvasWidth, canvasHeight) - actualBoardPixelSize) / 2);
  return { cellSize, padding, boardSize };
}

export function canvasToBoard(
  canvasX: number,
  canvasY: number,
  cfg: RenderConfig
): Position | null {
  const col = Math.round((canvasX - cfg.padding) / cfg.cellSize);
  const row = Math.round((canvasY - cfg.padding) / cfg.cellSize);
  if (col < 0 || col >= cfg.boardSize || row < 0 || row >= cfg.boardSize) return null;
  return { x: row, y: col };
}

export function boardToCanvas(pos: Position, cfg: RenderConfig): { x: number; y: number } {
  return {
    x: cfg.padding + pos.y * cfg.cellSize,
    y: cfg.padding + pos.x * cfg.cellSize,
  };
}

export function renderBoard(
  ctx: CanvasRenderingContext2D,
  board: CellState[][],
  cfg: RenderConfig,
  lastMove: Position | null
): void {
  const { cellSize, padding, boardSize } = cfg;
  const width = padding * 2 + (boardSize - 1) * cellSize;
  const height = width;

  // 背景 (木纹色)
  ctx.fillStyle = '#DEB887';
  ctx.fillRect(0, 0, width + padding, height + padding);

  // 棋盘区域
  ctx.fillStyle = '#F5DEB3';
  ctx.fillRect(padding - 10, padding - 10, (boardSize - 1) * cellSize + 20, (boardSize - 1) * cellSize + 20);

  // 网格线
  ctx.strokeStyle = '#8B7355';
  ctx.lineWidth = 1;
  for (let i = 0; i < boardSize; i++) {
    ctx.beginPath();
    ctx.moveTo(padding, padding + i * cellSize);
    ctx.lineTo(padding + (boardSize - 1) * cellSize, padding + i * cellSize);
    ctx.stroke();
    ctx.beginPath();
    ctx.moveTo(padding + i * cellSize, padding);
    ctx.lineTo(padding + i * cellSize, padding + (boardSize - 1) * cellSize);
    ctx.stroke();
  }

  // 星位 — 根据棋盘大小动态计算
  const starPoints = computeStarPoints(boardSize);
  ctx.fillStyle = '#8B7355';
  for (const [r, c] of starPoints) {
    if (r < boardSize && c < boardSize) {
      const { x, y } = boardToCanvas({ x: r, y: c }, cfg);
      ctx.beginPath();
      ctx.arc(x, y, 3, 0, Math.PI * 2);
      ctx.fill();
    }
  }

  // 棋子
  for (let x = 0; x < boardSize; x++) {
    for (let y = 0; y < boardSize; y++) {
      if (board[x]?.[y] === 0) continue;
      const { x: cx, y: cy } = boardToCanvas({ x, y }, cfg);
      const radius = cellSize * 0.43;

      if (board[x][y] === 1) {
        const gradient = ctx.createRadialGradient(cx - 2, cy - 2, 1, cx, cy, radius);
        gradient.addColorStop(0, '#4a4a4a');
        gradient.addColorStop(1, '#1a1a1a');
        ctx.fillStyle = gradient;
      } else {
        const gradient = ctx.createRadialGradient(cx - 2, cy - 2, 1, cx, cy, radius);
        gradient.addColorStop(0, '#ffffff');
        gradient.addColorStop(1, '#d0d0d0');
        ctx.fillStyle = gradient;
      }

      ctx.beginPath();
      ctx.arc(cx, cy, radius, 0, Math.PI * 2);
      ctx.fill();

      if (board[x][y] === 2) {
        ctx.strokeStyle = '#b0b0b0';
        ctx.lineWidth = 1;
        ctx.stroke();
      }
    }
  }

  // 最后一手高亮
  if (lastMove) {
    const { x, y } = boardToCanvas(lastMove, cfg);
    ctx.strokeStyle = '#ff4444';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(x, y, cellSize * 0.2, 0, Math.PI * 2);
    ctx.stroke();
  }
}
