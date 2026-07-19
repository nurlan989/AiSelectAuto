"""
/**
 * @brief End-to-end AI vehicle damage inspection Telegram bot
 *
 * This module implements a complete computer vision pipeline for automated
 * vehicle damage assessment via a Telegram interface.
 *
 * The system performs:
 * - image preprocessing (OpenCV + resizing + normalization)
 * - ONNX-based YOLO inference via ONNX Runtime
 * - post-processing (confidence filtering + decoding + bounding box scaling)
 * - optional visualization of detection results
 * - risk scoring based on aggregated defect analysis
 *
 * Architecture follows a simplified CV product pipeline:
 * UI (Telegram) → Preprocessing → Inference → Post-processing → Business logic
 *
 * Key characteristics:
 * - Stateless inference execution (ONNX Runtime session reuse)
 * - Lightweight in-memory user session tracking
 * - Rule-based risk scoring layer on top of CV output
 * - Designed for MVP-level deployment and validation
 *
 * This system is representative of real-world applied CV products
 * in insurance, automotive inspection, and marketplace evaluation domains.
 */
"""
import logging
import numpy as np
import cv2
import onnxruntime as ort
from collections import defaultdict
from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup
from telegram.ext import Application, CommandHandler, MessageHandler, CallbackQueryHandler, filters, ContextTypes

TOKEN = "8871457418:AAG88M4UfYu858TzFW7HHM42YYvIQxVf8HY"
MODEL_PATH = "/Users/nurlan/Documents/AI_Project/AISelectCar/models/best.onnx"
LABELS = [
    "Front-Windscreen-Damage",  # 0
    "Headlight-Damage",         # 1
    "Major-Rear-Bumper-Dent",   # 2
    "Rear-windscreen-Damage",   # 3
    "RunningBoard-Dent",        # 4
    "Sidemirror-Damage",        # 5
    "Signlight-Damage",         # 6
    "Taillight-Damage",         # 7
    "bonnet-dent",              # 8
    "doorouter-dent",           # 9
    "doorouter-scratch",        # 10
    "fender-dent",              # 11
    "front-bumper-dent",        # 12
    "front-bumper-scratch",     # 13
    "medium-Bodypanel-Dent",    # 14
    "paint-chip",               # 15
    "paint-trace",              # 16
    "pillar-dent",              # 17
    "quaterpanel-dent",         # 18
    "rear-bumper-dent",         # 19
    "rear-bumper-scratch",      # 20
    "roof-dent"                 # 21
]
CONF_THRESHOLD = 0.45
INPUT_SIZE = 640
PHOTO_LIMIT = 20

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

session = ort.InferenceSession(MODEL_PATH)
input_name = session.get_inputs()[0].name
output_name = session.get_outputs()[0].name

user_photo_count = defaultdict(int)
user_detections_total = defaultdict(list)

def run_inference(image_bytes):
    nparr = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    if img is None:
        return []
    orig_h, orig_w = img.shape[:2]
    scale_x = orig_w / INPUT_SIZE
    scale_y = orig_h / INPUT_SIZE
    resized = cv2.resize(img, (INPUT_SIZE, INPUT_SIZE))
    rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
    blob = rgb.astype(np.float32) / 255.0
    chw = np.transpose(blob, (2, 0, 1))
    tensor = np.expand_dims(chw, axis=0)
    output = session.run([output_name], {input_name: tensor})[0][0]
    num_classes = len(LABELS)
    detections = []
    if output.shape[0] == (4 + num_classes):
        for a in range(output.shape[1]):
            cx, cy, bw, bh = output[0,a], output[1,a], output[2,a], output[3,a]
            scores = output[4:, a]
            best_class = int(np.argmax(scores))
            confidence = float(scores[best_class])
            if confidence < CONF_THRESHOLD:
                continue
            x1 = int((cx - bw/2) * scale_x)
            y1 = int((cy - bh/2) * scale_y)
            x2 = int((cx + bw/2) * scale_x)
            y2 = int((cy + bh/2) * scale_y)
            detections.append({"label": LABELS[best_class], "confidence": confidence, "box": [x1, y1, x2, y2]})
    return detections

def draw_detections(image_bytes, detections):
    nparr = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    colors = {"dent": (0, 80, 255), "scratch": (0, 200, 80)}
    for d in detections:
        x1, y1, x2, y2 = d["box"]
        color = colors.get(d["label"], (255, 165, 0))
        label = f"{d['label']} {int(d['confidence']*100)}%"
        cv2.rectangle(img, (x1,y1), (x2,y2), color, 2)
        (tw,th),_ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.55, 2)
        cv2.rectangle(img, (x1,y1-th-8), (x1+tw+8,y1), color, -1)
        cv2.putText(img, label, (x1+4,y1-4), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255,255,255), 2)
    _, buf = cv2.imencode(".jpg", img)
    return buf.tobytes()

def get_risk(detections):
    count = len(detections)
    if count == 0:
        return "✅ Риск: НИЗКИЙ", "Автомобиль выглядит хорошо."
    elif count <= 2:
        return "✅ Риск: НИЗКИЙ", "Небольшие дефекты. Попробуйте скидку 2-5%."
    elif count <= 5:
        return "⚠️ Риск: СРЕДНИЙ", "Есть повреждения. Требуйте скидку 5-15%."
    else:
        return "🚨 Риск: ВЫСОКИЙ", "Много дефектов. Торгуйтесь или откажитесь."

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_photo_count[user_id] = 0
    user_detections_total[user_id] = []

    text = (
        "🚗 *AI Auto Check* — помощник при покупке авто\n\n"
        "Я анализирую фото и нахожу:\n"
        "• Вмятины и царапины\n"
        "• Повреждения бамперов и крыльев\n"
        "• Дефекты стёкол и фар\n\n"
        "📸 *Как фотографировать:*\n"
        "1️⃣ Перед авто 45° слева\n"
        "2️⃣ Перед авто 45° справа\n"
        "3️⃣ Левый бок полностью\n"
        "4️⃣ Правый бок полностью\n"
        "5️⃣ Зад авто 45° слева\n"
        "6️⃣ Зад авто 45° справа\n"
        "7️⃣ Капот сверху\n"
        "8️⃣ Под капотом\n\n"
        "💡 *Советы:*\n"
        "• Снимайте при дневном свете\n"
        "• Держите телефон горизонтально\n"
        "• Подходите близко к деталям\n\n"
        f"📊 У вас {PHOTO_LIMIT} бесплатных фото\n\n"
        "Отправьте первое фото!"
    )

    keyboard = [[InlineKeyboardButton("📋 Итоговый отчёт", callback_data="report")]]
    await update.message.reply_text(text, parse_mode="Markdown",
                                     reply_markup=InlineKeyboardMarkup(keyboard))

async def handle_photo(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id

    if user_photo_count[user_id] >= PHOTO_LIMIT:
        await update.message.reply_text(
            "❌ Вы использовали все 20 бесплатных фото.\n"
            "Напишите /start чтобы начать новую сессию."
        )
        return

    user_photo_count[user_id] += 1
    remaining = PHOTO_LIMIT - user_photo_count[user_id]

    await update.message.reply_text(
        f"🔍 Анализирую фото {user_photo_count[user_id]}/{PHOTO_LIMIT}...")

    photo = update.message.photo[-1]
    file = await context.bot.get_file(photo.file_id)
    image_bytes = bytes(await file.download_as_bytearray())

    detections = run_inference(image_bytes)
    user_detections_total[user_id].extend(detections)

    if not detections:
        await update.message.reply_text(
            f"✅ Дефектов не обнаружено.\n"
            f"📊 Осталось фото: {remaining}/{PHOTO_LIMIT}"
        )
        return

    result_image = draw_detections(image_bytes, detections)
    risk_label, risk_advice = get_risk(detections)

    report = f"📸 Фото {user_photo_count[user_id]}/{PHOTO_LIMIT}\n"
    report += f"⚠️ Найдено дефектов: {len(detections)}\n\n"
    for d in detections:
        report += f"• {d['label']} — {int(d['confidence']*100)}%\n"
    report += f"\n{risk_label}\n{risk_advice}"
    report += f"\n\n📊 Осталось фото: {remaining}/{PHOTO_LIMIT}"

    keyboard = [[InlineKeyboardButton("📋 Итоговый отчёт", callback_data="report")]]
    await context.bot.send_photo(
        chat_id=update.effective_chat.id,
        photo=result_image,
        caption=report,
        reply_markup=InlineKeyboardMarkup(keyboard)
    )

async def handle_report(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()
    user_id = query.from_user.id

    all_dets = user_detections_total[user_id]
    photos_checked = user_photo_count[user_id]

    if photos_checked == 0:
        await query.message.reply_text("Сначала отправьте фото!")
        return

    risk_label, risk_advice = get_risk(all_dets)
    label_counts = defaultdict(int)
    for d in all_dets:
        label_counts[d["label"]] += 1

    report = "📋 *Итоговый отчёт*\n\n"
    report += f"📸 Проверено фото: {photos_checked}\n"
    report += f"⚠️ Всего дефектов: {len(all_dets)}\n\n"

    if label_counts:
        report += "*Найденные дефекты:*\n"
        for label, count in sorted(label_counts.items(), key=lambda x: -x[1]):
            report += f"• {label}: {count} раз(а)\n"

    report += f"\n{risk_label}\n{risk_advice}\n\n"

    if len(all_dets) > 5:
        report += "💰 Требуйте скидку 10-20% или ремонт за счёт продавца."
    elif len(all_dets) > 2:
        report += "💰 Требуйте скидку 3-10% на ремонт."
    else:
        report += "💰 Авто в хорошем состоянии. Небольшой торг уместен."

    await query.message.reply_text(report, parse_mode="Markdown")

def main():
    app = Application.builder().token(TOKEN).build()
    app.add_handler(CommandHandler("start", start))
    app.add_handler(MessageHandler(filters.PHOTO, handle_photo))
    app.add_handler(CallbackQueryHandler(handle_report, pattern="report"))
    logger.info("Bot started!")
    app.run_polling()

if __name__ == "__main__":
    main()
