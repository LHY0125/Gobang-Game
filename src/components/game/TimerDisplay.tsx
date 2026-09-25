import { useState, useEffect, useRef, useCallback } from 'react';
import { useGameStore } from '../../store/gameStore';
import { invoke } from '@tauri-apps/api/core';

export default function TimerDisplay() {
  const config = useGameStore((s) => s.config);
  const currentColor = useGameStore((s) => s.currentColor);
  const status = useGameStore((s) => s.status);
  const refreshBoard = useGameStore((s) => s.refreshBoard);

  const blackTimeRef = useRef(config.timeLimitSecs);
  const whiteTimeRef = useRef(config.timeLimitSecs);
  const timerRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const hasTimedOutRef = useRef(false);
  const [, forceRender] = useState(0);

  // 初始化/重置时钟
  useEffect(() => {
    blackTimeRef.current = config.timeLimitSecs;
    whiteTimeRef.current = config.timeLimitSecs;
    hasTimedOutRef.current = false;
    forceRender((n) => n + 1);
  }, [config.timeLimitSecs, status]);

  const handleTimeout = useCallback(async () => {
    if (hasTimedOutRef.current) return;
    hasTimedOutRef.current = true;
    try {
      await invoke('resign');
      await refreshBoard();
    } catch {
      // 忽略错误
    }
  }, [refreshBoard]);

  useEffect(() => {
    if (!config.useTimer || status !== 'playing') {
      if (timerRef.current) {
        clearInterval(timerRef.current);
        timerRef.current = null;
      }
      return;
    }

    timerRef.current = setInterval(() => {
      if (currentColor === 'Black') {
        blackTimeRef.current -= 1;
        if (blackTimeRef.current <= 0) {
          blackTimeRef.current = 0;
          clearInterval(timerRef.current!);
          timerRef.current = null;
          handleTimeout();
        }
      } else {
        whiteTimeRef.current -= 1;
        if (whiteTimeRef.current <= 0) {
          whiteTimeRef.current = 0;
          clearInterval(timerRef.current!);
          timerRef.current = null;
          handleTimeout();
        }
      }
      forceRender((n) => n + 1);
    }, 1000);

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current);
        timerRef.current = null;
      }
    };
  }, [currentColor, config.useTimer, status, handleTimeout]);

  if (!config.useTimer) return null;

  const bTime = blackTimeRef.current;
  const wTime = whiteTimeRef.current;
  const displayTime = currentColor === 'Black' ? bTime : wTime;
  const isWarning = displayTime <= 10;

  return (
    <div className="timer-display">
      <div className={isWarning ? 'timer-warning' : ''} style={{ fontSize: 28, fontFamily: 'monospace' }}>
        {Math.floor(displayTime / 60)}:{(displayTime % 60).toString().padStart(2, '0')}
      </div>
      <div style={{ display: 'flex', gap: 20, fontSize: 14, opacity: 0.7 }}>
        <span style={{ fontWeight: currentColor === 'Black' ? 'bold' : 'normal' }}>
          黑: {Math.floor(bTime / 60)}:{String(bTime % 60).padStart(2, '0')}
        </span>
        <span style={{ fontWeight: currentColor === 'White' ? 'bold' : 'normal' }}>
          白: {Math.floor(wTime / 60)}:{String(wTime % 60).padStart(2, '0')}
        </span>
      </div>
    </div>
  );
}
