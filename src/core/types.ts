export type Color = 'Black' | 'White';

export interface Position {
  x: number;
  y: number;
}

export type CellState = 0 | 1 | 2; // 0=Empty, 1=Black, 2=White

export type GameStatus = 'waiting' | 'playing' | 'ai_thinking' | 'game_over';

export type GameModeType = 'Local' | 'VsAi' | 'Online' | 'Replay';

export interface GameConfig {
  boardSize: number;
  useForbiddenRules: boolean;
  useTimer: boolean;
  timeLimitSecs: number;
  aiDifficulty: number;
  playerColor: Color;
  isServer: boolean;
  remoteAddress: string;
}

export interface MoveResult {
  position: Position;
  is_win: boolean;
  is_forbidden: boolean;
}

export interface Move {
  position: Position;
  color: Color;
  turn: number;
}
