# Beacon Playback Interfaces

## Overview

This directory contains abstract interfaces for the Beacon playback application.  
Interfaces here define the contract for message classification, playback rules, and message sending logic.

---

## Key Concepts

### Authorities

**Definition:**  
Authorities are rule engines that control the flow, timing, and outcome of market data messages during playback.

**Role:**  
- Decide if a message should be sent, dropped, delayed, vetoed, or modified.
- Enforce safety, control, timing, and chaos rules.

**Example Authorities:**
- **BurstAuthority:** Sends bursts of messages at intervals.
- **ChaosAuthority:** Randomly drops or delays messages to simulate network chaos.
- **RateLimitAuthority:** Limits the rate of message transmission.

**Why messages may be VETO'd, dropped, or allowed:**  
- **VETO:** A rule determines that a message must be held or not sent (e.g., safety violation, sequence error).
- **DROP:** A rule decides the message should be discarded (e.g., chaos simulation, invalid data).
- **SEND:** All rules allow the message to be sent immediately or after a delay.

---

### Advisors

**Definition:**  
Advisors are classifiers or analyzers that provide metadata or advice about messages, such as priority or burst detection.

**Role:**  
- Analyze messages and assign priorities or detect bursts.
- Do not directly control message flow, but their advice can influence authorities.

**Example Advisors:**
- **MessagePriorityAdvisor:** Classifies messages as NORMAL, ELEVATED, CRITICAL, or EMERGENCY.
- **BurstDetectionAdvisor:** Identifies bursts in message streams.

---

### Rules

**Definition:**  
Rules are the logic implemented by authorities to evaluate each message and determine its fate.

**Role:**  
- Encapsulate decision-making logic (e.g., timing, chaos, safety).
- Can be composed and prioritized.

**Example Rule Outcomes:**
- **CONTINUE:** Message passes all checks, proceed as normal.
- **SEND_NOW:** Message should be sent immediately.
- **DROP:** Message should be discarded.
- **VETO:** Message is held or blocked by a rule.
- **MODIFIED:** Message is altered before sending.

---

## Message Outcomes Explained

- **VETO:**  
  Used when a rule (often safety or control) determines a message must not be sent.  
  Example: Sequence violation, risk limit exceeded.

- **DROP:**  
  Used when a rule (often chaos or filter) decides the message should be discarded.  
  Example: Simulating packet loss, filtering out invalid data.

- **SEND_NOW / CONTINUE:**  
  Used when all rules allow the message to be sent, possibly after a delay.  
  Example: Normal operation, timing rule satisfied.

- **MODIFIED:**  
  Used when a rule alters the message before sending.  
  Example: Adjusting timestamps, anonymizing data.

---

## Usage

- Implement interfaces in this directory to create new authorities or advisors.
- Configure which rules and advisors are active via playback configuration files.
- Authorities and advisors work together to provide robust, flexible playback control.

---

## Example Workflow

1. Advisors classify messages (e.g., assign priority).
2. Authorities evaluate each message using rules.
3. Each rule can VETO, DROP, DELAY, MODIFY, or ALLOW the message.
4. Playback engine sends, holds, or discards messages based on rule outcomes.

---

**For more details, see the implementation files and playback documentation.**
