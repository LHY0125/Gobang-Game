import { describe, it, expect, vi, beforeEach } from 'vitest';

// Mock Tauri invoke before importing the store
vi.mock('@tauri-apps/api/core', () => ({
  invoke: vi.fn(),
}));

import { invoke } from '@tauri-apps/api/core';
import { useGameStore, buildReplayBoard } from '../gameStore';

const mockInvoke = invoke as ReturnType<typeof vi.fn>;

function resetStore() {
  useGameStore.setState({
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
  });
}

beforeEach(() => {
  mockInvoke.mockReset();
  resetStore();
});

describe('gameStore', () => {
  describe('startGame', () => {
    it('calls new_game with correct params and sets playing status', async () => {
      mockInvoke.mockResolvedValueOnce(undefined);
      mockInvoke.mockResolvedValueOnce({
        board: [],
        current_color: 'Black',
        game_over: false,
        winner: null,
      });

      const config = { ...useGameStore.getState().config };
      await useGameStore.getState().startGame('Local', config);

      expect(mockInvoke).toHaveBeenCalledWith('new_game', { mode: 'Local', config });
      const state = useGameStore.getState();
      expect(state.mode).toBe('Local');
      expect(state.status).toBe('playing');
      expect(state.currentColor).toBe('Black');
      expect(state.winner).toBeNull();
    });

    it('sets ai_thinking when player chooses White in VsAi mode', async () => {
      mockInvoke.mockResolvedValueOnce(undefined);
      mockInvoke.mockResolvedValueOnce({
        board: [],
        current_color: 'Black',
        game_over: false,
        winner: null,
      });

      const config = { ...useGameStore.getState().config, playerColor: 'White' as const };
      await useGameStore.getState().startGame('VsAi', config);

      expect(useGameStore.getState().status).toBe('ai_thinking');
    });

    it('sets playing when player chooses Black in VsAi mode', async () => {
      mockInvoke.mockResolvedValueOnce(undefined);
      mockInvoke.mockResolvedValueOnce({
        board: [],
        current_color: 'Black',
        game_over: false,
        winner: null,
      });

      const config = { ...useGameStore.getState().config, playerColor: 'Black' as const };
      await useGameStore.getState().startGame('VsAi', config);

      expect(useGameStore.getState().status).toBe('playing');
    });
  });

  describe('placePiece', () => {
    it('sets winner and game_over on winning move', async () => {
      mockInvoke.mockResolvedValueOnce({
        position: { x: 7, y: 7 },
        is_win: true,
        is_forbidden: false,
      });
      mockInvoke.mockResolvedValueOnce({
        board: [],
        current_color: 'White',
        game_over: true,
        winner: 'Black',
      });

      useGameStore.setState({ status: 'playing', currentColor: 'Black' });
      const result = await useGameStore.getState().placePiece(7, 7);

      expect(result.is_win).toBe(true);
      expect(useGameStore.getState().status).toBe('game_over');
      expect(useGameStore.getState().winner).toBe('Black');
    });
  });

  describe('aiMove', () => {
    it('calls ai_move_llm when useLlm is true', async () => {
      useGameStore.setState({
        config: { ...useGameStore.getState().config, useLlm: true },
      });
      mockInvoke.mockResolvedValueOnce([7, 7]); // ai_move_llm result
      mockInvoke.mockResolvedValueOnce({
        position: { x: 7, y: 7 },
        is_win: false,
        is_forbidden: false,
      }); // place_piece
      mockInvoke.mockResolvedValueOnce({
        board: [],
        current_color: 'White',
        game_over: false,
        winner: null,
      }); // get_game_state

      await useGameStore.getState().aiMove();

      expect(mockInvoke).toHaveBeenCalledWith('ai_move_llm');
    });
  });

  describe('buildReplayBoard', () => {
    it('rebuilds board to specified step', () => {
      const moves = [
        { position: { x: 7, y: 7 }, color: 'Black' as const, turn: 0 },
        { position: { x: 7, y: 8 }, color: 'White' as const, turn: 1 },
        { position: { x: 6, y: 7 }, color: 'Black' as const, turn: 2 },
        { position: { x: 6, y: 8 }, color: 'White' as const, turn: 3 },
      ];

      const board = buildReplayBoard(15, moves, 2);

      expect(board[7][7]).toBe(1); // Black
      expect(board[7][8]).toBe(2); // White
      expect(board[6][7]).toBe(0); // not yet placed
    });

    it('handles step larger than moves length', () => {
      const moves = [
        { position: { x: 7, y: 7 }, color: 'Black' as const, turn: 0 },
      ];
      const board = buildReplayBoard(15, moves, 5);
      expect(board[7][7]).toBe(1);
    });

    it('handles empty moves', () => {
      const board = buildReplayBoard(15, [], 0);
      for (let x = 0; x < 15; x++) {
        for (let y = 0; y < 15; y++) {
          expect(board[x][y]).toBe(0);
        }
      }
    });
  });
});
