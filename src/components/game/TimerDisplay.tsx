import { useState, useEffect } from 'react';
import { useGameStore } from '../../store/gameStore';

export default function TimerDisplay() {
  const config = useGameStore((s) => s.config);
  const currentColor = useGameStore((s) => s.currentColor);
  const status = useGameStore((s) => s.status);
  const [time, setTime] = useState(config.timeLimitSecs);

  useEffect(() => {
    if (!config.useTimer || status !== 'playing') return;
    setTime(config.timeLimitSecs);
    const timer = setInterval(() => {
      setTime((t) => {
        if (t <= 1) { clearInterval(timer); return 0; }
        return t - 1;
      });
    }, 1000);
    return () => clearInterval(timer);
  }, [currentColor, config.useTimer, config.timeLimitSecs, status]);

  if (!config.useTimer) return null;

  return (
    <div className={`timer-display ${time <= 10 ? 'timer-warning' : ''}`}>
      {Math.floor(time / 60)}:{(time % 60).toString().padStart(2, '0')}
    </div>
  );
}
