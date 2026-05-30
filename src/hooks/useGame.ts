import { useCallback } from 'react';
import { useGameStore } from '../store/gameStore';
import type { GameConfig, GameModeType } from '../core/types';

export function useGame() {
  const store = useGameStore();

  const startGame = useCallback(async (mode: GameModeType, config: GameConfig) => {
    await store.startGame(mode, config);
  }, [store]);

  return { ...store, startGame };
}
