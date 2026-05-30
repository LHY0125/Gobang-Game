import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';
import type { CellState, Color, GameConfig, GameModeType, GameStatus, Move, MoveResult } from '../core/types';

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

  startGame: (mode: GameModeType, config: GameConfig) => Promise<void>;
  placePiece: (x: number, y: number) => Promise<MoveResult>;
  undo: (steps?: number) => Promise<void>;
  aiMove: () => Promise<void>;
  refreshBoard: () => Promise<void>;
  loadReplayBoard: (board: CellState[][], moves: Move[]) => void;
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
  },
  isSaving: false,

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
    });
    await get().refreshBoard();
  },

  placePiece: async (x, y) => {
    const result: MoveResult = await invoke('place_piece', { x, y });
    await get().refreshBoard();
    if (result.is_win) {
      set({ status: 'game_over' });
    }
    return result;
  },

  undo: async (steps = 1) => {
    await invoke('undo', { steps });
    await get().refreshBoard();
  },

  aiMove: async () => {
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
  },

  refreshBoard: async () => {
    const state: { board: CellState[][]; current_color: string; game_over: boolean } =
      await invoke('get_game_state');
    set({
      board: state.board,
      currentColor: state.current_color as Color,
      status: state.game_over ? 'game_over' : get().status === 'ai_thinking' ? 'ai_thinking' : 'playing',
    });
  },

  loadReplayBoard: (board, moves) => {
    set({ board, moves, mode: 'Replay', status: 'playing' });
  },
}));
