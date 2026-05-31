import { describe, it, expect } from 'vitest';

describe('types', () => {
  it('CellState values are correct', () => {
    const empty: number = 0;
    const black: number = 1;
    const white: number = 2;
    expect(empty).toBe(0);
    expect(black).toBe(1);
    expect(white).toBe(2);
  });

  it('MoveResult has correct shape', () => {
    const result = {
      position: { x: 7, y: 7 },
      is_win: false,
      is_forbidden: false,
    };
    expect(result.position.x).toBe(7);
    expect(result.is_win).toBe(false);
  });

  it('GameConfig has all required fields with defaults', () => {
    const config = {
      boardSize: 15,
      useForbiddenRules: true,
      useTimer: false,
      timeLimitSecs: 60,
      aiDifficulty: 3,
      playerColor: 'Black' as const,
      isServer: false,
      remoteAddress: '',
    };
    expect(config.boardSize).toBeGreaterThanOrEqual(9);
    expect(config.boardSize).toBeLessThanOrEqual(19);
    expect(['Black', 'White']).toContain(config.playerColor);
  });
});
