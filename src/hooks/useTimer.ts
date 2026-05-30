import { useState, useEffect } from 'react';

export function useTimer(seconds: number, active: boolean, onTimeout: () => void) {
  const [time, setTime] = useState(seconds);

  useEffect(() => {
    if (!active) return;
    setTime(seconds);
    const timer = setInterval(() => {
      setTime((t) => {
        if (t <= 1) { clearInterval(timer); onTimeout(); return 0; }
        return t - 1;
      });
    }, 1000);
    return () => clearInterval(timer);
  }, [active, seconds, onTimeout]);

  return time;
}
