import { useState, useEffect, useRef, useCallback } from 'react';
import { useGameStore } from '../../store/gameStore';
import { invoke } from '@tauri-apps/api/core';

export default function TimerDisplay() {
  const config = useGameStore((s) => s.config);
  const currentColor = useGameStore((s) => s.currentColor);
  const status = useGameStore((s) => s.status);
  const refreshBoard = useGameStore((s) => s.refreshBoard);

  const [blackTime, setBlackTime] = useState(config.timeLimitSecs);
  const [whiteTime, setWhiteTime] = useState(config.timeLimitSecs);
  const lastColorRef = useRef(currentColor);
  const hasTimedOutRef = useRef(false);

  // 初始化/重置时钟
  useEffect(() => {
    setBlackTime(config.timeLimitSecs);
    setWhiteTime(config.timeLimitSecs);
    hasTimedOutRef.current = false;
    lastColorRef.current = 'Black';
  }, [config.timeLimitSecs, status === 'waiting' ? status : null]);

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
    if (!config.useTimer || status !== 'playing') return;

    const timer = setInterval(() => {
      if (currentColor === 'Black') {
        setBlackTime((t) => {
          if (t <= 1) {
            clearInterval(timer);
            handleTimeout();
            return 0;
          }
          return t - 1;
        });
      } else {
        setWhiteTime((t) => {
          if (t <= 1) {
            clearInterval(timer);
            handleTimeout();
            return 0;
          }
          return t - 1;
        });
      }
    }, 1000);

    lastColorRef.current = currentColor;

    return () => clearInterval(timer);
  }, [currentColor, config.useTimer, status, handleTimeout]);

  if (!config.useTimer) return null;

  const displayTime = currentColor === 'Black' ? blackTime : whiteTime;
  const isWarning = displayTime <= 10;

  return (
    <div className="timer-display">
      <div className={isWarning ? 'timer-warning' : ''} style={{ fontSize: 28, fontFamily: 'monospace' }}>
        {Math.floor(displayTime / 60)}:{(displayTime % 60).toString().padStart(2, '0')}
      </div>
      <div style={{ display: 'flex', gap: 20, fontSize: 14, opacity: 0.7 }}>
        <span style={{ fontWeight: currentColor === 'Black' ? 'bold' : 'normal' }}>
          黑: {Math.floor(blackTime / 60)}:{String(blackTime % 60).padStart(2, '0')}
        </span>
        <span style={{ fontWeight: currentColor === 'White' ? 'bold' : 'normal' }}>
          白: {Math.floor(whiteTime / 60)}:{String(whiteTime % 60).padStart(2, '0')}
        </span>
      </div>
    </div>
  );
}
