import { describe, it, expect } from 'vitest';
import {
  computeBoardDimensions,
  canvasToBoard,
  boardToCanvas,
  computeStarPoints,
} from '../board-renderer';

describe('computeBoardDimensions', () => {
  it('returns positive cellSize and padding for 15x15 board', () => {
    const cfg = computeBoardDimensions(15, 800, 600);
    expect(cfg.cellSize).toBeGreaterThan(0);
    expect(cfg.padding).toBeGreaterThan(0);
    expect(cfg.boardSize).toBe(15);
  });

  it('fits within the smaller canvas dimension', () => {
    const cfg = computeBoardDimensions(15, 800, 600);
    const total = cfg.padding * 2 + (cfg.boardSize - 1) * cfg.cellSize;
    expect(total).toBeLessThanOrEqual(800);
  });
});

describe('canvasToBoard / boardToCanvas round-trip', () => {
  it('round-trips for a 15x15 board center', () => {
    const cfg = computeBoardDimensions(15, 800, 600);
    const boardPos = { x: 7, y: 7 };
    const canvas = boardToCanvas(boardPos, cfg);
    const restored = canvasToBoard(canvas.x, canvas.y, cfg);
    expect(restored).toEqual(boardPos);
  });

  it('returns null for clicks outside the board', () => {
    const cfg = computeBoardDimensions(15, 800, 600);
    expect(canvasToBoard(-10, -10, cfg)).toBeNull();
    expect(canvasToBoard(9999, 9999, cfg)).toBeNull();
  });
});

describe('computeStarPoints', () => {
  it('returns 9 star points for 15x15 board', () => {
    const points = computeStarPoints(15);
    expect(points.length).toBe(9);
  });

  it('returns only center for board smaller than 9', () => {
    const points = computeStarPoints(7);
    expect(points.length).toBe(1);
    expect(points[0]).toEqual([3, 3]);
  });

  it('star points are within board bounds', () => {
    for (const size of [9, 13, 15, 19]) {
      const points = computeStarPoints(size);
      for (const [r, c] of points) {
        expect(r).toBeGreaterThanOrEqual(0);
        expect(r).toBeLessThan(size);
        expect(c).toBeGreaterThanOrEqual(0);
        expect(c).toBeLessThan(size);
      }
    }
  });
});
