import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';
import type { CellState, Color, GameConfig, GameModeType, GameStatus, Move, MoveResult } from '../core/types';

/** 根据落子列表重建棋盘到指定步数 */
export function buildReplayBoard(boardSize: number, moves: Move[], step: number): CellState[][] {
  const b: CellState[][] = Array.from({ length: boardSize }, () => Array(boardSize).fill(0) as CellState[]);
  const limit = Math.min(step, moves.length);
  for (let i = 0; i < limit; i++) {
    const m = moves[i];
    b[m.position.x][m.position.y] = (m.color === 'Black' ? 1 : 2) as CellState;
  }
  return b;
}

interface GameState {
  mode: GameModeType;
  board: CellState[][];
  boardSize: number;
  currentColor: Color;
  status: GameStatus;
  winner: Color | null;
  moves: Move[];
  config: GameConfig;
  isSaving: boolean;
  replayStep: number;
  llmThinking: string;

  startGame: (mode: GameModeType, config: GameConfig) => Promise<void>;
  placePiece: (x: number, y: number) => Promise<MoveResult>;
  undo: (steps?: number) => Promise<void>;
  aiMove: () => Promise<void>;
  refreshBoard: () => Promise<void>;
  loadReplayBoard: (board: CellState[][], moves: Move[]) => void;
  setReplayStep: (step: number) => void;
  appendLlmThinking: (chunk: string) => void;
  clearLlmThinking: () => void;
}

export const useGameStore = create<GameState>((set, get) => ({
  mode: 'Local',
  board: [],
  boardSize: 15,
  currentColor: 'Black',
  status: 'waiting',
  winner: null,
  moves: [],
  config: {
    boardSize: 15,
    useForbiddenRules: true,
    useTimer: false,
    timeLimitSecs: 60,
    aiDifficulty: 3,
    playerColor: 'Black',
    isServer: false,
    remoteAddress: '',
  },
  isSaving: false,
  replayStep: 0,
  llmThinking: '',

  startGame: async (mode, config) => {
    await invoke('new_game', { mode, config });
    set({
      mode,
      config,
      boardSize: config.boardSize,
      status: mode === 'VsAi' && config.playerColor === 'White' ? 'ai_thinking' : 'playing',
      currentColor: 'Black',
      winner: null,
      moves: [],
      replayStep: 0,
      llmThinking: '',
    });
    await get().refreshBoard();
  },

  placePiece: async (x, y) => {
    const { currentColor } = get();
    const result: MoveResult = await invoke('place_piece', { x, y });
    await get().refreshBoard();
    if (result.is_win) {
      set({ status: 'game_over', winner: currentColor });
    }
    return result;
  },

  undo: async (steps = 1) => {
    await invoke('undo', { steps });
    await get().refreshBoard();
  },

  aiMove: async () => {
    const { config } = get();
    if (config.useLlm) {
      set({ status: 'ai_thinking', llmThinking: '' });
      const pos: [number, number] | null = await invoke('ai_move_llm');
      if (pos) {
        const result = await get().placePiece(pos[0], pos[1]);
        if (!result.is_win) {
          set({ status: 'playing' });
        }
      } else {
        set({ status: 'playing' });
      }
    } else {
      set({ status: 'ai_thinking' });
      const pos: [number, number] | null = await invoke('ai_move');
      if (pos) {
        const result = await get().placePiece(pos[0], pos[1]);
        if (!result.is_win) {
          set({ status: 'playing' });
        }
      } else {
        set({ status: 'playing' });
      }
    }
  },

  refreshBoard: async () => {
    const state: { board: CellState[][]; current_color: string; game_over: boolean; winner: string | null } =
      await invoke('get_game_state');
    const newStatus: GameStatus = state.game_over
      ? 'game_over'
      : get().status === 'ai_thinking'
        ? 'ai_thinking'
        : 'playing';
    set({
      board: state.board,
      currentColor: state.current_color as Color,
      status: newStatus,
      winner: state.winner as Color | null,
    });
  },

  loadReplayBoard: (board, moves) => {
    set({ board, moves, mode: 'Replay', status: 'playing', replayStep: moves.length });
  },

  setReplayStep: (step) => {
    set({ replayStep: step });
  },

  appendLlmThinking: (chunk) => {
    set((s) => ({ llmThinking: s.llmThinking + chunk }));
  },

  clearLlmThinking: () => {
    set({ llmThinking: '' });
  },
}));
