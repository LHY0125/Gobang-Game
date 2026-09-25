export type Color = 'Black' | 'White';

export interface Position {
  x: number;
  y: number;
}

export type CellState = 0 | 1 | 2; // 0=Empty, 1=Black, 2=White

export type GameStatus = 'waiting' | 'playing' | 'ai_thinking' | 'game_over';

export type GameModeType = 'Local' | 'VsAi' | 'Online' | 'Replay';

export interface GameRulesConfig {
  boardSize: number;
  useForbiddenRules: boolean;
  useTimer: boolean;
  timeLimitSecs: number;
}

export interface AiConfig {
  aiDifficulty: number;
  playerColor: Color;
  useLlm?: boolean;
  llmEndpoint?: string;
  llmApiKey?: string;
  llmModel?: string;
}

export interface NetworkConfig {
  isServer: boolean;
  remoteAddress: string;
  hostPort?: number;
}

/** 游戏总配置（扁平 JSON，与 Rust GameConfig #[serde(flatten)] 对齐） */
export interface GameConfig extends GameRulesConfig, AiConfig, NetworkConfig {}

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
