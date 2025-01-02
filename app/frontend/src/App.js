import React, { useState, useEffect } from 'react';
import axios from 'axios';
import './App.css';

const App = () => {
  const [boards, setBoards] = useState([]);

  // Fetch all board data from the backend
  const fetchBoards = async () => {
    try {
      const response = await axios.get('http://localhost:5000/api/boards');
      setBoards(response.data);
    } catch (err) {
      console.error('Failed to fetch board data:', err);
    }
  };

  useEffect(() => {
    fetchBoards();
    const interval = setInterval(fetchBoards, 5000); // Refresh data every 5 seconds
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="App">
      <h1>Sensor Data Dashboard</h1>
      <div className="board-container">
        {boards.map((board) => (
          <div key={board.boardId} className="board">
            <h3>Board ID: {board.boardId}</h3>
            <table>
              <thead>
                <tr>
                  <th>Timestamp</th>
                  <th>Temperature (°C)</th>
                  <th>Pressure (hPa)</th>
                </tr>
              </thead>
              <tbody>
                {board.readings.map((reading, index) => (
                  <tr key={index}>
                    <td>{new Date(reading.timestamp).toLocaleString()}</td>
                    <td>{reading.temperature}</td>
                    <td>{reading.pressure}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        ))}
      </div>
    </div>
  );
};

export default App;
